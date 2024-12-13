#pragma once

#include <optional>
#include <regex>

#include "nanosvg.h"
#include "rack.hpp"

using namespace rack;

template <typename T>
struct SvgHelper {


    std::shared_ptr<Svg> svg;

    ModuleWidget* moduleWidget() {
        auto t = static_cast<T*>(this);
        return static_cast<ModuleWidget*>(t);
    }

    void loadPanel(const std::string& filename) {
        if (svg == nullptr) {
            DEBUG("Loading SVG file [%s]", filename.c_str());
            // Load and parse the SVG file for the first time.
            auto panel = createPanel(filename);
            svg = panel->svg;
            moduleWidget()->setPanel(panel);
        } else if (svg != nullptr) {
            DEBUG("Reloading SVG file [%s]", filename.c_str());
            // Once loaded, VCV Rack caches the panel internally.
            // We have to force it to reload and reparse the SVG file.
            // Attempt to create a new SVG handle before replacing the one
            // that exists. This way, in case the file is missing or corrupt,
            // we don't lose the existing panel, nor do we risk crashing VCV Rack.
            // This is quite likely during iterative development, which is why
            // this code exists in the first place!
            NSVGimage* replacement =
                nsvgParseFromFile(filename.c_str(), "px", SVG_DPI);

            if (replacement == nullptr) {
                // Leave the existing panel in place, and log why it didn't change.
                WARN("Cannot load/parse SVG file [%s]", filename.c_str());
            } else {
                // Successful reload. Destroy the old SVG and replace it with the new one.
                if (svg->handle) {
                    DEBUG("Deleting old SVG handle");
                    nsvgDelete(svg->handle);
                }

                DEBUG("Replacing SVG handle");
                svg->handle = replacement;
                rack::ModuleWidget* widget = moduleWidget();
                SvgPanel* panel = dynamic_cast<SvgPanel*>(widget->getPanel());
                panel->fb->dirty = true;
            }
        } else {
            // This should never happen. If it does, there is a bug I need to fix.
            WARN("Weird! Somehow we lost our SVG panel.");
        }
    }

    void forEachShape(const std::function<void(NSVGshape*)>& callback) {
        auto shapes = svg->handle->shapes;
        for (NSVGshape* shape = shapes; shape != nullptr; shape = shape->next) {
            callback(shape);
        }
    }

    // return the actual shape
    std::optional<NSVGshape*> findShape(std::string name) {
        NSVGshape* result = nullptr;

        forEachShape([&](NSVGshape* shape) {
            if (std::string(shape->id) == name) {
                result = shape;
                return;
            }
        });

        return result;
    }

    std::optional<Vec> findNamed(std::string name) {
        std::optional<Vec> result;

        forEachShape([&](NSVGshape* shape) {
            DEBUG("Shape id: %s", shape->id);
            if (std::string(shape->id) == name) {
                auto bounds = shape->bounds;
                result = Vec();
                result->x = (bounds[0] + bounds[2]) / 2;
                result->y = (bounds[1] + bounds[3]) / 2;
                return;
            }
        });

        return result;
    }

    std::vector<Vec> findPrefixed(std::string prefix) {
        std::vector<Vec> result;

        forEachShape([&](NSVGshape* shape) {
            if (std::string(shape->id).find(prefix) == 0) {
                auto bounds = shape->bounds;
                auto center =
                    Vec((bounds[0] + bounds[2]) / 2,
                        (bounds[1] + bounds[3]) / 2);
                result.push_back(center);
            }
        });

        return result;
    }

    std::vector<std::pair<std::vector<std::string>, Vec>> findMatched(
        const std::string& pattern
    ) {
        std::regex regex(pattern);
        std::vector<std::pair<std::vector<std::string>, Vec>> result;

        forEachShape([&](NSVGshape* shape) {
            auto id = std::string(shape->id);

            std::vector<std::string> captures;
            std::smatch match;

            if (std::regex_search(id, match, regex)) {
                for (unsigned int i = 1; i < match.size(); i++) {
                    captures.push_back(match[i]);
                }
                auto bounds = shape->bounds;
                auto center =
                    Vec((bounds[0] + bounds[2]) / 2,
                        (bounds[1] + bounds[3]) / 2);
                result.emplace_back(captures, center);
            }
        });

        return result;
    }

    void forEachPrefixed(
        std::string prefix,
        const std::function<void(unsigned int i, Vec)>& callback
    ) {
        auto positions = findPrefixed(prefix);
        for (unsigned int i = 0; i < positions.size(); i++) {
            callback(i, positions[i]);
        }
    }

    void forEachMatched(
        const std::string& regex,
        const std::function<void(std::vector<std::string>, Vec)>& callback
    ) {
        auto matches = findMatched(regex);
        for (const auto& match : matches) {
            callback(match.first, match.second);
        }
    }
};
#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <map>
#include <regex>

#include "nanosvg.h"
#include "rack.hpp"

using namespace rack;

template <typename T>
struct SvgHelper {

   private:
    bool devMode = false;
    bool pollMode = false;
    double prevPollTime = 0.0;
    struct stat prevStatBuf;
    std::string svgFileName;

    app::SvgPanel* panel = nullptr;
    std::map<std::string, Widget*> widgets;

    ModuleWidget* widget() {
        auto t = static_cast<T*>(this);
        return static_cast<ModuleWidget*>(t);
    }

    void addReloadableLight(std::string svgid, LightWidget* light) {
        widget()->addChild(light);
        widgets[svgid] = light;
    }

    void addReloadableOutput(std::string svgid, PortWidget* output) {
        widget()->addOutput(output);
        widgets[svgid] = output;
    }

    void addReloadableInput(std::string svgid, PortWidget* input) {
        widget()->addInput(input);
        widgets[svgid] = input;
    }

    void addReloadableParam(std::string svgid, ParamWidget* param) {
        widget()->addParam(param);
        widgets[svgid] = param;
    }

    void addReloadableChild(std::string svgid, Widget* child) {
        widget()->addChild(child);
        widgets[svgid] = child;
    }

   public:
    Vec calculateCenter(NSVGshape* shape) {
        auto bounds = shape->bounds;
        return Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
    }

    void setDevMode(bool devMode) { this->devMode = devMode; }
    void setDirty() {
        if (panel && panel->fb)
            panel->fb->dirty = true;
    }

    void appendContextMenu(Menu* menu) {
        if (!devMode)
            return;

        if (panel == nullptr || svgFileName.empty()) {
            return;
        }

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Reload panel", "", [this]() { loadPanel(this->svgFileName); }));
        menu->addChild(createBoolPtrMenuItem<bool>("Poll SVG for reload", "", &pollMode));
    }

    void loadPanel(const std::string& filename) {
        if (panel == nullptr) {
            DEBUG("Loading SVG file [%s]", filename.c_str());
            // Load and parse the SVG file for the first time.
            svgFileName = filename;
            panel = createPanel(filename);
            widget()->setPanel(panel);
        } else {
            DEBUG("Reloading SVG file [%s]", filename.c_str());
            NSVGimage* replacement = nsvgParseFromFile(filename.c_str(), "px", SVG_DPI);

            if (replacement == nullptr) {
                // Leave the existing panel in place, and log why it didn't change.
                WARN("Cannot load/parse SVG file [%s]", filename.c_str());
            } else {
                svgFileName = filename;

                // Successful reload. Destroy the old SVG and replace it with the new one.
                if (panel->svg->handle) {
                    DEBUG("Deleting old SVG handle");
                    nsvgDelete(panel->svg->handle);
                }

                DEBUG("Replacing SVG handle");
                panel->svg->handle = replacement;
                setDirty();
            }

            if (panel && panel->svg && panel->svg->handle) {
                // Find shapes whose SVG identifier matches one of our control names.
                // Use coordinates from the SVG object to set the position of the matching control.
                forEachShape([&](NSVGshape* shape) {
                    auto search = widgets.find(shape->id);
                    if (search != widgets.end())
                        reposition(search->second, shape);
                });

                setDirty();
            }
        }
    }

    void forEachShape(const std::function<void(NSVGshape*)>& callback) {
        auto shapes = panel->svg->handle->shapes;
        for (NSVGshape* shape = shapes; shape != nullptr; shape = shape->next) {
            callback(shape);
        }
    }

    // return the actual shape
    NSVGshape* findNamed(std::string name) {
        NSVGshape* result = nullptr;

        forEachShape([&](NSVGshape* shape) {
            if (std::string(shape->id) == name) {
                result = shape;
                return;
            }
        });

        return result;
    }

    std::vector<NSVGshape*> findPrefixed(std::string prefix) {
        std::vector<NSVGshape*> result;

        forEachShape([&](NSVGshape* shape) {
            if (std::string(shape->id).find(prefix) == 0) {
                result.push_back(shape);
            }
        });

        return result;
    }

    std::vector<std::pair<std::vector<std::string>, NSVGshape*>> findMatched(const std::string& pattern) {
        std::regex regex(pattern);
        std::vector<std::pair<std::vector<std::string>, NSVGshape*>> result;

        forEachShape([&](NSVGshape* shape) {
            auto id = std::string(shape->id);

            std::vector<std::string> captures;
            std::smatch match;

            if (std::regex_search(id, match, regex)) {
                for (unsigned int i = 1; i < match.size(); i++) {
                    captures.push_back(match[i]);
                }
                result.emplace_back(captures, shape);
            }
        });

        return result;
    }

    void forEachPrefixed(std::string prefix, const std::function<void(unsigned int i, NSVGshape*)>& callback) {
        auto positions = findPrefixed(prefix);
        for (unsigned int i = 0; i < positions.size(); i++) {
            callback(i, positions[i]);
        }
    }

    void forEachMatched(const std::string& regex,
        const std::function<void(std::vector<std::string>, NSVGshape*)>& callback) {
        auto matches = findMatched(regex);
        for (const auto& match : matches) {
            callback(match.first, match.second);
        }
    }

    // reload support
    void reposition(Widget* widget, NSVGshape* shape) {
        float x = (shape->bounds[0] + shape->bounds[2]) / 2;
        float y = (shape->bounds[1] + shape->bounds[3]) / 2;
        widget->box.pos = Vec{x, y}.minus(widget->box.size.div(2));
    }

    template <class TParamWidget>
    void bindParam(NSVGshape* shape, int id) {
        auto pos = calculateCenter(shape);
        addReloadableParam(shape->id, createParamCentered<TParamWidget>(pos, widget()->module, id));
    }

    template <class TParamWidget>
    void bindParam(std::string name, int id) {
        if (auto* shape = findNamed(name)) {
            bindParam<TParamWidget>(shape, id);
        }
    }

    template <class TPortWidget>
    void bindInput(NSVGshape* shape, int id) {
        auto pos = calculateCenter(shape);
        addReloadableInput(shape->id, createInputCentered<TPortWidget>(pos, widget()->module, id));
    }

    template <class TPortWidget>
    void bindInput(std::string name, int id) {
        if (auto* shape = findNamed(name)) {
            bindInput<TPortWidget>(shape, id);
        }
    }

    template <class TPortWidget>
    void bindOutput(NSVGshape* shape, int id) {
        auto pos = calculateCenter(shape);
        addReloadableOutput(shape->id, createOutputCentered<TPortWidget>(pos, widget()->module, id));
    }

    template <class TPortWidget>
    void bindOutput(std::string name, int id) {
        if (auto* shape = findNamed(name)) {
            bindOutput<TPortWidget>(shape, id);
        }
    }

    template <class TLightWidget>
    void bindLight(NSVGshape* shape, int id) {
        auto pos = calculateCenter(shape);
        addReloadableLight(shape->id, createLightCentered<TLightWidget>(pos, widget()->module, id));
    }

    template <class TLightWidget>
    void bindLight(std::string svgid, int id) {
        if (auto* shape = findNamed(svgid)) {
            bindLight<TLightWidget>(shape, id);
        }
    }

    template <class TWidget>
    void bindChild(NSVGshape* shape) {
        auto pos = calculateCenter(shape);
        addReloadableChild(shape->id, createWidgetCentered<TWidget>(pos));
    }

    template <class TWidget>
    void bindChild(std::string name) {
        if (auto* shape = findNamed(name)) {
            bindChild<TWidget>(shape);
        }
    }

    void step() {
        if (pollMode && !svgFileName.empty()) {
            // Check the SVG file's last-modification-time once per second.
            double now = system::getTime();
            double elapsed = now - prevPollTime;
            if (elapsed >= 1.0) {
                prevPollTime = now;
                struct stat statBuf;
                if (0 == stat(svgFileName.c_str(), &statBuf)) {
                    // Has the SVG file's modification time changed?
                    // For maximum source portability, check the POSIX-required seconds field only.
                    // We aren't polling more than once per second, and realistically,
                    // the SVG file isn't changing more than once per second either.
                    if (0 != memcmp(&statBuf.st_mtime, &prevStatBuf.st_mtime, sizeof(statBuf.st_mtime))) {
                        prevStatBuf = statBuf;
                        loadPanel(svgFileName);
                    }
                }
            }
        }
    }
};
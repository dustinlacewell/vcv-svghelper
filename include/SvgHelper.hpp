#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <map>
#include <regex>

#include "app/SvgPanel.hpp"
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
    std::string lightSvgFileName;
    std::string darkSvgFileName;

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

        if (panel == nullptr || lightSvgFileName.empty()) {
            return;
        }

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Reload panel", "", [this]() { reloadPanel(); }));
        menu->addChild(createBoolPtrMenuItem("Poll SVG for reload", "", &pollMode));
    }

    // Original method for backwards compatibility
    void loadPanel(const std::string& filename) {
        lightSvgFileName = filename;
        darkSvgFileName = "";

        if (panel == nullptr) {
            DEBUG("Loading SVG file [%s]", filename.c_str());
            panel = createPanel(filename);
            widget()->setPanel(panel);
        } else {
            reloadPanel();
        }
    }

    // New method supporting dark mode
    void loadPanel(const std::string& lightFilename, const std::string& darkFilename) {
        lightSvgFileName = lightFilename;
        darkSvgFileName = darkFilename;

        if (panel == nullptr) {
            DEBUG("Loading SVG files - Light: [%s], Dark: [%s]", lightFilename.c_str(), darkFilename.c_str());
            panel = createPanel(lightFilename, darkFilename);
            widget()->setPanel(panel);
        } else {
            reloadPanel();
        }
    }
    void forEachShape(const std::function<void(NSVGshape*)>& callback) {
        if (!panel || !panel->svg || !panel->svg->handle)
            return;
        auto shapes = panel->svg->handle->shapes;
        for (NSVGshape* shape = shapes; shape != nullptr; shape = shape->next) {
            callback(shape);
        }
    }

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
        if (pollMode && !lightSvgFileName.empty()) {
            double now = system::getTime();
            double elapsed = now - prevPollTime;
            if (elapsed >= 1.0) {
                prevPollTime = now;
                struct stat statBuf;
                bool needsReload = false;

                if (0 == stat(lightSvgFileName.c_str(), &statBuf)) {
                    if (0 != memcmp(&statBuf.st_mtime, &prevStatBuf.st_mtime, sizeof(statBuf.st_mtime))) {
                        needsReload = true;
                    }
                }

                if (!darkSvgFileName.empty()) {
                    if (0 == stat(darkSvgFileName.c_str(), &statBuf)) {
                        if (0 != memcmp(&statBuf.st_mtime, &prevStatBuf.st_mtime, sizeof(statBuf.st_mtime))) {
                            needsReload = true;
                        }
                    }
                }

                if (needsReload) {
                    prevStatBuf = statBuf;
                    reloadPanel();
                }
            }
        }
    }

   private:
    void reloadPanel() {
        DEBUG("Reloading SVG files");

        if (darkSvgFileName.empty()) {
            // Single panel mode
            NSVGimage* replacement = nsvgParseFromFile(lightSvgFileName.c_str(), "px", SVG_DPI);
            if (replacement) {
                if (panel->svg->handle) {
                    nsvgDelete(panel->svg->handle);
                }
                panel->svg->handle = replacement;
            } else {
                WARN("Cannot load/parse SVG file [%s]", lightSvgFileName.c_str());
                return;
            }
        } else {
            // Themed panel mode
            auto* themedPanel = dynamic_cast<app::ThemedSvgPanel*>(panel);
            if (!themedPanel)
                return;

            NSVGimage* lightReplacement = nsvgParseFromFile(lightSvgFileName.c_str(), "px", SVG_DPI);
            NSVGimage* darkReplacement = nsvgParseFromFile(darkSvgFileName.c_str(), "px", SVG_DPI);

            if (lightReplacement) {
                if (themedPanel->lightSvg->handle) {
                    nsvgDelete(themedPanel->lightSvg->handle);
                }
                themedPanel->lightSvg->handle = lightReplacement;
            }

            if (darkReplacement) {
                if (themedPanel->darkSvg->handle) {
                    nsvgDelete(themedPanel->darkSvg->handle);
                }
                themedPanel->darkSvg->handle = darkReplacement;
            }
        }

        // Update widget positions using light SVG as reference
        forEachShape([&](NSVGshape* shape) {
            auto search = widgets.find(shape->id);
            if (search != widgets.end())
                reposition(search->second, shape);
        });

        setDirty();
    }
};
#pragma once

#include <optional>
#include <regex>

#include "nanosvg.h"
#include "rack.hpp"

using namespace rack;

struct SvgHelper {
    std::shared_ptr<Svg> svg;
    ModuleWidget* moduleWidget;

    explicit SvgHelper(ModuleWidget* moduleWidget);

    void setPanel(const std::string& filename);

    std::optional<Vec> findNamed(std::string name);
    std::vector<Vec> findPrefixed(std::string prefix);
    std::vector<std::pair<std::vector<std::string>, Vec>> findMatched(const std::string& regex);

    void forEachPrefixed(std::string prefix, const std::function<void(unsigned int i, Vec)>& callback);
    void forEachMatched(const std::string& regex,
        const std::function<void(std::vector<std::string> captures, Vec)>& callback);
    void forEachShape(const std::function<void(NSVGshape*)>& callback);
};

#pragma once

#include <regex>

#include "nanosvg.h"
#include "rack.hpp"

using namespace rack;

struct SvgHelper {

    std::string filename;

    SvgHelper(std::string filename);

    std::vector<Vec> findPrefixed(std::string prefix);
    std::vector<std::pair<std::vector<std::string>, Vec>> findMatched(std::string regex);

    void forEachPrefixed(std::string prefix, std::function<void(int i, Vec)> callback);
    void forEachMatched(std::string regex, std::function<void(std::vector<std::string> captures, Vec)> callback);
};

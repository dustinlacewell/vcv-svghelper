#pragma once

#include <regex>

#include <rack.hpp>
#include "nanosvg.h"

using namespace rack;

struct SvgHelper {

    std::string filename;

    SvgHelper(std::string filename);

    std::vector<Vec> findPrefixed(std::string prefix);
    std::vector<std::pair<std::vector<std::string>, Vec>> findMatched(std::string regex);

    void forEachPrefixed(std::string prefix, std::function<void(int i, Vec)> callback);
    void forEachMatched(std::string regex, std::function<void(std::vector<std::string> captures, Vec)> callback);
};

SvgHelper::SvgHelper(std::string filename) : filename(filename) {}

std::vector<Vec> SvgHelper::findPrefixed(std::string prefix) {
    std::vector<Vec> result;

    NSVGimage* image;
    image = nsvgParseFromFile(filename.c_str(), "mm", 96);

    // Use...
    for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
        if (std::string(shape->id).find(prefix) == 0) {
            auto bounds = shape->bounds;
            auto center = Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
            result.push_back(center);
        }
    }
    // Delete
    nsvgDelete(image);

    return result;
}

std::vector<std::pair<std::vector<std::string>, Vec>> SvgHelper::findMatched(std::string pattern) {
std::vector<std::pair<std::vector<std::string>, Vec>> result;

std::regex regex(pattern);

NSVGimage* image;
image = nsvgParseFromFile(filename.c_str(), "mm", 96);

for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
// match regular expression against shape->id
// if match, add to result
std::vector<std::string> captures;

auto id = std::string(shape->id);

// iterate all the groups in the regex
std::smatch match;

if (std::regex_search(id, match, regex)) {
for (int i = 1; i < match.size(); i++) {
captures.push_back(match[i]);
}
auto bounds = shape->bounds;
auto center = Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
result.push_back(std::make_pair(captures, center));
}
}

// Delete
nsvgDelete(image);

return result;
}

void SvgHelper::forEachPrefixed(std::string prefix, std::function<void(int i, Vec)> callback) {
    auto positions = findPrefixed(prefix);
    for (int i = 0; i < positions.size(); i++) {
        callback(i, positions[i]);
    }
}

void SvgHelper::forEachMatched(std::string regex, std::function<void(std::vector<std::string>, Vec)> callback) {
    auto matches = findMatched(regex);
    for (auto match : matches) {
        callback(match.first, match.second);
    }
}
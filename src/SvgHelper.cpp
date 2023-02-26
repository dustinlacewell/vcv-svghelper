#include "SvgHelper.hpp"

SvgHelper::SvgHelper(std::string filename) : filename(filename) {}

NSVGimage* loadImage(std::string filename) {
    return nsvgParseFromFile(filename.c_str(), "mm", 96);
}

void withImage(std::string filename, std::function<void(NSVGimage*)> callback) {
    auto image = loadImage(filename);
    callback(image);
    nsvgDelete(image);
}

Vec SvgHelper::findNamed(std::string name) {
    Vec result;

    withImage(filename, [&](NSVGimage* image) {
        for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
            if (std::string(shape->id) == name) {
                auto bounds = shape->bounds;
                result = Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
                return;
            }
        }
    });

    return result;
}

std::vector<Vec> SvgHelper::findPrefixed(std::string prefix) {
    std::vector<Vec> result;

    withImage(filename, [&](NSVGimage* image) {
        for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
            if (std::string(shape->id).find(prefix) == 0) {
                auto bounds = shape->bounds;
                auto center = Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
                result.push_back(center);
            }
        }
    });

    return result;
}

std::vector<std::pair<std::vector<std::string>, Vec>> SvgHelper::findMatched(std::string pattern) {
    std::vector<std::pair<std::vector<std::string>, Vec>> result;

    withImage(filename, [&](NSVGimage* image) {
        std::regex regex(pattern);

        for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
            auto id = std::string(shape->id);

            std::vector<std::string> captures;
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
    });

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
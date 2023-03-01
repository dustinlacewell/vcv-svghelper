#include "SvgHelper.hpp"

#include <utility>

SvgHelper::SvgHelper(ModuleWidget* moduleWidget) : moduleWidget(moduleWidget) {}

void SvgHelper::setPanel(const std::string& filename) {
    if (svg == nullptr) {
        auto panel = createPanel(filename);
        svg = panel->svg;
        moduleWidget->setPanel(panel);
    } else {
        // Once loaded, VCV Rack caches the panel internally.
        // We have to force it to reload the file.
        try {
            svg->loadFile(filename);
        } catch (Exception& e) {
            WARN("Cannot reload panel from %s: %s", filename.c_str(), e.what());
        }
    }
}

void SvgHelper::forEachShape(const std::function<void(NSVGshape*)>& callback) {
    auto shapes = svg->handle->shapes;
    for (NSVGshape* shape = shapes; shape != nullptr; shape = shape->next) {
        callback(shape);
    }
}

std::optional<Vec> SvgHelper::findNamed(std::string name) {
    std::optional<Vec> result;

    forEachShape([&](NSVGshape* shape) {
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

std::vector<Vec> SvgHelper::findPrefixed(std::string prefix) {
    std::vector<Vec> result;

    forEachShape([&](NSVGshape* shape) {
        if (std::string(shape->id).find(prefix) == 0) {
            auto bounds = shape->bounds;
            auto center = Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
            result.push_back(center);
        }
    });

    return result;
}

std::vector<std::pair<std::vector<std::string>, Vec>> SvgHelper::findMatched(const std::string& pattern) {
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
            auto center = Vec((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
            result.emplace_back(captures, center);
        }
    });

    return result;
}

void SvgHelper::forEachPrefixed(std::string prefix, const std::function<void(unsigned int i, Vec)>& callback) {
    auto positions = findPrefixed(prefix);
    for (unsigned int i = 0; i < positions.size(); i++) {
        callback(i, positions[i]);
    }
}

void SvgHelper::forEachMatched(const std::string& regex,
    const std::function<void(std::vector<std::string>, Vec)>& callback) {
    auto matches = findMatched(regex);
    for (const auto& match : matches) {
        callback(match.first, match.second);
    }
}
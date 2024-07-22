#include "MetaModule.h"

MetaModule::MetaModule(const std::string& filename) {
    int x = 0, y = 0;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        for (char c : line) {
            if (c == '#') {
                coords.push_back({x, y});
            }
            x++;
        }
        x = 0;
        y++;
    }
    metaModules.push_back(coords);
    file.close();
}

void MetaModule::generateRotations() {
    std::vector<std::pair<int, int>> rotation = coords;
    for (int j = 0; j < coords.size(); j++) {
        std::swap(rotation[j].first, rotation[j].second);
    }
    metaModules.push_back(rotation);
}

void MetaModule::generateReflections() {
    std::vector<std::vector<std::pair<int, int>>> newMetaModules;
    std::vector<std::pair<int, int>> reflection;
    for (int j = 0; j < metaModules.size(); j++) {
        reflection = metaModules[j];
        for (int k = 0; k < reflection.size(); k++) {
            reflection[k].first *= -1;
        }
        newMetaModules.push_back(reflection);
    }
    for (auto& metaModule : newMetaModules) {
        metaModules.push_back(metaModule);
    }
}

void MetaModule::printCoordsOnly() const {
    for (int i = 0; i < metaModules.size(); ++i) {
        std::cout << "Configuration " << i << " coordinates:\n";
        for (const auto& coord : metaModules[i]) {
            std::cout << "(" << coord.first << ", " << coord.second << ")\n";
        }
        std::cout << "\n";
    }
}

void MetaModule::printConfigurations() const {
    const int width = 10;
    const int height = 10;
    const int offset = 5;
    for (int i = 0; i < metaModules.size(); ++i) {
        std::cout << "Configuration " << i << ":\n";
        char grid[width][height];
        for (int m = 0; m < width; ++m) {
            for (int n = 0; n < height; ++n) {
                grid[m][n] = '.';
            }
        }
        for (const auto& coord : metaModules[i]) {
            if ((coord.second + offset) < width && (coord.first + offset) < height) {
                grid[coord.second + offset][coord.first + offset] = '#';
            }
        }
        for (int y = 0; y < width; ++y) {
            for (int x = 0; x < height; ++x) {
                std::cout << grid[y][x] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

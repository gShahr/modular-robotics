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
    file.close();
}

MetaModule* MetaModule::MakeCopy() const {
    auto copy = new MetaModule(*this);
    return copy;
}

void MetaModule::Rotate(int index) {
    for (auto& coord : coords) {
        std::swap(coord[0], coord[index]);
    }
}

void MetaModule::Reflect(int index) {
    for (auto& coord : coords) {
        coord[index] *= -1;
    }
}

void MetaModule::printCoordsOnly() const {
    for (int i = 0; i < coords.size(); ++i) {
        std::cout << "Configuration " << i << " coordinates:\n";
        for (const auto& coord : coords[i]) {
            std::cout << coord << ' ';
        }
        std::cout << "\n";
    }
}

void MetaModule::printConfigurations() const {
    const int width = 10;
    const int height = 10;
    const int offset = 5;
    for (int i = 0; i < coords.size(); i++) {
        std::cout << "Configuration " << i << ":\n";
        char grid[width][height];
        for (int m = 0; m < width; ++m) {
            for (int n = 0; n < height; ++n) {
                grid[m][n] = '.';
            }
        }
        grid[coords[i][0] + offset][coords[i][1] + offset] = '#';
        for (int y = 0; y < width; ++y) {
            for (int x = 0; x < height; ++x) {
                std::cout << grid[y][x] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

namespace MetaModuleManager {
    std::vector<MetaModule> metamodules;
}
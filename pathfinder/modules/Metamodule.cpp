#include "Metamodule.h"

MetaModule::MetaModule(const std::string& filename, int order, int size) : order(order), size(size) {
    readFromJson(filename);
}

MetaModule* MetaModule::MakeCopy() const {
    auto copy = new MetaModule(*this);
    return copy;
}

void MetaModule::readFromTxt2d(const std::string& filename) {
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
                coords.emplace_back(-1, std::valarray<int>(x, y));
            }
            x++;
        }
        x = 0;
        y++;
    }
    file.close();
}

void MetaModule::readFromJson(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return;
    }
    nlohmann::json j;
    file >> j;
    for (const auto& module : j["modules"]) {
        std::vector<int> position = module["position"];
        std::transform(position.begin(), position.end(), position.begin(),
                    [](int coord) { return coord; });
        std::valarray<int> coords(position.data(), position.size());
        if (module.contains("color")) {
            this->coords.emplace_back(Colors::colorToInt[module["color"]], coords);
        } else {
            this->coords.emplace_back(-1, coords);
        }
    }
}

void MetaModule::Rotate(int a, int b) {
    for (auto& coord : coords) {
        std::swap(coord.second[a], coord.second[b]);
    }
}

void MetaModule::Reflect(int index) {
    for (auto& coord : coords) {
        coord.second[index] *= -1;
        // Ternary needs testing
        //coord[index] += size - 1;
        coord.second[index] += size - size % 2;
    }
}

void MetaModule::printCoordsOnly() const {
    for (size_t i = 0; i < coords.size(); ++i) {
        std::cout << "Configuration " << i << " coordinates:\n";
        for (const auto& coord : coords[i].second) {
            std::cout << coord << ' ';
        }
        std::cout << "\n";
    }
}

void MetaModule::printConfigurations() const {
    const int width = 10;
    const int height = 10;
    const int offset = 5;
    for (size_t i = 0; i < coords.size(); i++) {
        std::cout << "Configuration " << i << ":\n";
        char grid[width][height];
        for (int m = 0; m < width; ++m) {
            for (int n = 0; n < height; ++n) {
                grid[m][n] = '.';
            }
        }
        grid[coords[i].second[0] + offset][coords[i].second[1] + offset] = '#';
        for (int y = 0; y < width; ++y) {
            for (int x = 0; x < height; ++x) {
                std::cout << grid[y][x] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

std::vector<MetaModule*> MetaModuleManager::metamodules = {};
int MetaModuleManager::order = 0;
int MetaModuleManager::axisSize = 0;

void MetaModuleManager::InitMetaModuleManager(int _order, int _axisSize) {
    order = _order;
    axisSize = _axisSize;
}

void MetaModuleManager::GenerateFrom(MetaModule* metamodule) {
    auto list = Isometry::GenerateTransforms(metamodule);
    for (auto metamodule: list) {
        metamodules.push_back(dynamic_cast<MetaModule*>(metamodule));
    }
}

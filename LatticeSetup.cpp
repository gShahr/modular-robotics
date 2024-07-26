#include "LatticeSetup.h"
#include "ConfigurationSpace.h"
#include "MetaModule.h"
#include "Colors.h"
#include <set>

namespace LatticeSetup {
    void setupFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            return;
        }
        nlohmann::json j;
        file >> j;
        Lattice::InitLattice(j["order"], j["axisSize"]);
        std::set<int> colors;
        for (const auto& module : j["modules"]) {
            std::vector<int> position = module["position"];
            std::transform(position.begin(), position.end(), position.begin(),
                        [](int coord) { return coord; });
            std::valarray<int> coords(position.data(), position.size());
            if (!Lattice::ignoreColors && module.contains("color")) {
                Lattice::AddModule(coords, module["static"], module["color"]);
                colors.insert(Color::colorToInt[module["color"]]);
            } else {
                Lattice::AddModule(coords, module["static"]);
            }
        }
        if (colors.size() <= 1) {
            Lattice::colorTensor = CoordTensor<int>(0, 0, 0);
            Lattice::ignoreColors = true;
        }
        Lattice::BuildMovableModules();
    }

    Configuration setupFinalFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            throw std::ios_base::failure("Unable to open file " + filename + "\n");
        }
        nlohmann::json j;
        file >> j;
        CoordTensor<bool> desiredState(Lattice::Order(), Lattice::AxisSize(), false);
        CoordTensor<int> desiredColors(Lattice::Order(), Lattice::AxisSize(), -1);
        for (const auto& module : j["modules"]) {
            std::vector<int> position = module["position"];
            std::transform(position.begin(), position.end(), position.begin(),
                        [](int coord) { return coord; });
            std::valarray<int> coords(position.data(), position.size());
            desiredState[coords] = true;
            if (!Lattice::ignoreColors && module.contains("color")) {
                desiredColors[coords] = Color::colorToInt[module["color"]];
            }
        }
        return {desiredState, desiredColors};
    }

    void setupInitial(const std::string& filename, int order, int axisSize) {
        Lattice::InitLattice(order, axisSize);
        std::vector<std::vector<char>> image;
        int x = 0;
        int y = 0;
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            for (char c : line) {
                if (c == '1') {
                    std::valarray<int> coords = {x, y};
                    Lattice::AddModule(coords);
                } else if (c == '@') {
                    std::valarray<int> coords = {x, y};
                    Lattice::AddModule(coords, true);
                }
                x++;
            }
            x = 0;
            y++;
        }
        Lattice::BuildMovableModules();
    }

    Configuration setupFinal(const std::string& filename) {
        int x = 0;
        int y = 0;
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            throw std::ios_base::failure("Unable to open file " + filename + "\n");
        }
        CoordTensor<bool> desiredState(Lattice::Order(), Lattice::AxisSize(), false);
        CoordTensor<int> colors(Lattice::Order(), Lattice::AxisSize(), -1);
        std::string line;
        while (std::getline(file, line)) {
            for (char c : line) {
                if (c == '1') {
                    std::valarray<int> coords = {x, y};
                    desiredState[coords] = true;
                }
                x++;
            }
            x = 0;
            y++;
        }
        return Configuration(desiredState, colors);
    }

    void setUpMetamodule(MetaModule* metamodule) {
        Lattice::InitLattice(metamodule->order, metamodule->size);
        for (const auto &coord: metamodule->coords) {
            Lattice::AddModule(coord);
        }
    }

    void setUpTiling() {
        Lattice::InitLattice(MetaModuleManager::order, MetaModuleManager::axisSize);
        for (int i = 0; i < MetaModuleManager::axisSize / MetaModuleManager::metamodules[0]->size; i++) {
            for (int j = 0; j < MetaModuleManager::axisSize / MetaModuleManager::metamodules[0]->size; j++) {
                if ((i%2==0 && j&1) || (i&1 && j%2 == 0)) {
                    for (const auto &coord: MetaModuleManager::metamodules[5]->coords) {
                        std::valarray<int> newCoord = {MetaModuleManager::metamodules[5]->size * i, MetaModuleManager::metamodules[5]->size * j};
                        Lattice::AddModule(coord + newCoord);
                    }
                } else {
                    for (const auto &coord: MetaModuleManager::metamodules[0]->coords) {
                        std::valarray<int> newCoord = {MetaModuleManager::metamodules[0]->size * i, MetaModuleManager::metamodules[0]->size * j};
                        Lattice::AddModule(coord + newCoord);
                    }
                }
            }
        }
        // for (const auto &coord: MetaModuleManager::metamodules[0]->coords) {
        //     Lattice::AddModule(coord);
        // }
        // for (const auto &coord: MetaModuleManager::metamodules[5]->coords) {
        //     std::valarray<int> newCoord = {3, 0};
        //     Lattice::AddModule(coord + newCoord);
        // }
        // for (const auto &coord: MetaModuleManager::metamodules[0]->coords) {
        //     std::valarray<int> newCoord = {3, 3};
        //     Lattice::AddModule(coord + newCoord);
        // }
        // for (const auto &coord: MetaModuleManager::metamodules[5]->coords) {
        //     std::valarray<int> newCoord = {0, 3};
        //     Lattice::AddModule(coord + newCoord);
        // }
    }
};
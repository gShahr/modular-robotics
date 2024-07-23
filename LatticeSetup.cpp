#include "LatticeSetup.h"
#include "MetaModule.h"

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
        for (const auto& module : j["modules"]) {
            std::vector<int> position = module["position"];
            std::transform(position.begin(), position.end(), position.begin(),
                        [](int coord) { return coord; });
            std::valarray<int> coords(position.data(), position.size());
            Lattice::AddModule(coords, module["static"]);
        }
        Lattice::BuildMovableModules();
    }

    CoordTensor<bool> setupFinalFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            return CoordTensor<bool>(1, 1, false);
        }
        nlohmann::json j;
        file >> j;
        CoordTensor<bool> desiredState(Lattice::Order(), Lattice::AxisSize(), false);
        for (const auto& module : j["modules"]) {
            std::vector<int> position = module["position"];
            std::transform(position.begin(), position.end(), position.begin(),
                        [](int coord) { return coord; });
            std::valarray<int> coords(position.data(), position.size());
            desiredState[coords] = true;
        }
        return desiredState;

    }

    void setupInitial(const std::string& filename) {
        Lattice::InitLattice(2, 9);
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

    CoordTensor<bool> setupFinal(const std::string& filename) {
        int x = 0;
        int y = 0;
        CoordTensor<bool> desiredState(Lattice::Order(), Lattice::AxisSize(), false);
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            return desiredState;
        }
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
        return desiredState;
    }

    void setUpMetamodule(MetaModule* metamodule) {
        Lattice::InitLattice(2, 10);
        for (const auto &coord: metamodule->coords) {
            Lattice::AddModule(coord);
        }
    }

    void setUpTiling() {
        Lattice::InitLattice(MetaModuleManager::order, MetaModuleManager::axisSize);
        for (int i = 0; i < MetaModuleManager::axisSize / MetaModuleManager::metamodules[0]->axisSize; i++) {
            for (int j = 0; j < MetaModuleManager::axisSize / MetaModuleManager::metamodules[0]->axisSize; j++) {
                int flag = 0;
                if ((i%2==0 && j&1) || (i&1 && j%2 == 0)) {
                    for (const auto &coord: MetaModuleManager::metamodules[5]->coords) {
                        std::valarray<int> newCoord = {MetaModuleManager::metamodules[5]->axisSize * i, MetaModuleManager::metamodules[5]->axisSize * j};
                        Lattice::AddModule(coord + newCoord);
                    }
                } else {
                    for (const auto &coord: MetaModuleManager::metamodules[0]->coords) {
                        std::valarray<int> newCoord = {MetaModuleManager::metamodules[0]->axisSize * i, MetaModuleManager::metamodules[0]->axisSize * j};
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
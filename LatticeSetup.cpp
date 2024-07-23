#include "LatticeSetup.h"

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

    void setupInitial(const std::string& filename) {
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
};
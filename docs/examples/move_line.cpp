#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <fstream>
#include <set>
#include <string>
#include "MoveManager.h"
#include "ConfigurationSpace.h"
#include "debug_util.h"
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <queue>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include "MetaModule.h"
#include "LatticeSetup.h"
#include "Scenario.h"

int main() {
    int order = 2;
    int axisSize = 9;
    std::string folder = "docs/examples/";
    Lattice lattice(order, axisSize);
    MoveManager::InitMoveManager(order, axisSize);
    LatticeSetup::setupFromJson(lattice, folder + "move_line_initial.json");
    std::cout << lattice;
    MoveManager::RegisterAllMoves();
    std::cout << "BFS Testing:\n";
    std::cout << "Original:    Desired:\n" <<
                 "  ----         --##\n" <<
                 "  -#--         ---#\n" <<
                 "  -##-         ----\n" <<
                 "  ----         ----\n";
    Configuration start(lattice.stateTensor);
    CoordTensor<bool> desiredState = LatticeSetup::setupFinal(order, axisSize, lattice, folder + "move_line_final.txt");
    Configuration end(desiredState);
    auto path = ConfigurationSpace::BFS(&start, &end, lattice);
    std::cout << "Path:\n";
    for (auto config : path) {
        lattice = config->GetState();
        std::cout << lattice;
    }
    Scenario::exportToScen(lattice, path, folder + "move_line_scen.scen");
    MoveManager::CleanMoves();
    return 0;
}
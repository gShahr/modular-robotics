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
    // Set up Lattice
    LatticeSetup::setupFromJson("docs/examples/move_line_initial.json");

    // Set up moves
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());

    std::cout << Lattice::ToString();
    MoveManager::RegisterAllMoves();
    /*std::cout << "Attempting to assign to lattice from state tensor.\n";
    CoordTensor<bool> stateTest(order, axisSize, false);
    stateTest[{1, 1}] = true;
    stateTest[{1, 2}] = true;
    stateTest[{2, 2}] = true;
    lattice = stateTest;
    std::cout << lattice;*/

    // BFS TESTING
    std::cout << "BFS Testing:\n";
    Configuration start(Lattice::stateTensor);
    CoordTensor<bool> desiredState = LatticeSetup::setupFinal("docs/examples/move_line_final.txt");
    Configuration end(desiredState);
    auto path = ConfigurationSpace::BFS(&start, &end);
    std::cout << "Path:\n";
    for (auto config : path) {
        Lattice::UpdateFromState(config->GetState());
        std::cout << Lattice::ToString();
    }
    std::string exportFolder = "Visualization/Scenarios/";
    Scenario::exportToScen(path, exportFolder + "test.scen");

    // Cleanup
    MoveManager::CleanMoves();
    return 0;
}


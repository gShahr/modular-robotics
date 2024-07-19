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
    Lattice lattice(order, axisSize);
    MoveManager::InitMoveManager(order, axisSize);
    //setupInitial(ORIGIN, lattice, "test1.txt");
    LatticeSetup::setupFromJson(lattice, "test1.json");

    std::cout << lattice;
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
    std::cout << "Original:    Desired:\n" <<
                 "  ----         --##\n" <<
                 "  -#--         ---#\n" <<
                 "  -##-         ----\n" <<
                 "  ----         ----\n";
    Configuration start(lattice.stateTensor);
    CoordTensor<bool> desiredState = LatticeSetup::setupFinal(order, axisSize, lattice, "test1DesiredState.txt");

    // desiredState[{3,3}] = false;
    // desiredState[{4,3}] = false;
    // desiredState[{4,4}] = false;
    // desiredState[{4,5}] = false;
    // desiredState[{5,5}] = false;
    // desiredState[{8,3}] = true;
    // desiredState[{8,4}] = true;
    // desiredState[{7,4}] = true;
    // desiredState[{6,4}] = true;
    // desiredState[{6,5}] = true;

    /*desiredState[{0,1}] = false;
    desiredState[{0,2}] = false;
    desiredState[{0,3}] = false;
    desiredState[{0,4}] = false;
    desiredState[{1,5}] = true;
    desiredState[{2,5}] = true;
    desiredState[{3,5}] = true;
    desiredState[{4,5}] = true;*/
    Configuration end(desiredState);
    auto path = ConfigurationSpace::BFS(&start, &end, lattice);
    std::cout << "Path:\n";
    for (auto config : path) {
        lattice = config->GetState();
        std::cout << lattice;
    }
    Scenario::exportToScen(lattice, path, "test.scen");

    // Cleanup
    MoveManager::CleanMoves();
    return 0;
}


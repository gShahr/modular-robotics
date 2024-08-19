#include <random>
#include <cassert>
#include <chrono>
#include <iostream>
#include <getopt.h>
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

Configuration generateRandomConfiguration() {
    std::string initialFile = "docs/examples/move_nofinal_initial.json";
    LatticeSetup::setupFromJson(initialFile);
    Configuration config(Lattice::GetModuleInfo());
    return config; //ConfigurationSpace::GenerateRandomFinal();
}

int main(int argc, char* argv[]) {    
    bool ignoreColors = false;
    Lattice::setFlags(ignoreColors);
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves();
    std::string initialFile = "docs/examples/move_nofinal_initial.json";
    LatticeSetup::setupFromJson(initialFile);
    Configuration start(Lattice::GetModuleInfo());
    int tests = 10;
    for (int i = 0; i < tests; i++) {
        Configuration end = generateRandomConfiguration();
        auto aStarPath = ConfigurationSpace::AStar(&start, &end);
        auto bfsPath = ConfigurationSpace::BFS(&start, &end);
        std::cout << "Test " << i << std::endl;
        if (aStarPath.size() != bfsPath.size()) {
            std::cerr << "Error: A* path size does not match BFS path size for test " << i << std::endl;
        }
    }
    std::cout << "AStar test passed " << tests <<  " tests" << std::endl;
    return 0;
}
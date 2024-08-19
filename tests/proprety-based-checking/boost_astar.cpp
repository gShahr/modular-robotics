#define BOOST_TEST_MODULE AStar
#include <boost/test/included/unit_test.hpp>
#include <random>
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
    return ConfigurationSpace::GenerateRandomFinal();
}

BOOST_AUTO_TEST_CASE(testAStar) {
    std::random_device rd;
    std::mt19937 gen(rd());
    bool ignoreColors = false;
    Lattice::setFlags(ignoreColors);
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves();
    std::string initialFile = "docs/examples/move_nofinal_initial.json";
    LatticeSetup::setupFromJson(initialFile);
    Configuration start(Lattice::GetModuleInfo());
    for (int i = 0; i < 100; ++i) { // Generate 100 random test cases
        Configuration end = generateRandomConfiguration();
        auto aStarPath = ConfigurationSpace::AStar(&start, &end);
        auto bfsPath = ConfigurationSpace::BFS(&start, &end);
        BOOST_CHECK(aStarPath == bfsPath);
    }
}

// BOOST_AUTO_TEST_SUITE_END()
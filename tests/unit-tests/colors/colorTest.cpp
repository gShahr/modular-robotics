#define BOOST_TEST_MODULE IgnoreColorTest
#include <boost/test/included/unit_test.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <queue>
#include <unordered_set>
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <nlohmann/json.hpp>
#include "../../../pathfinder/lattice/LatticeSetup.h"
#include "../../../pathfinder/moves/Scenario.h"
#include "../../../pathfinder/moves/MoveManager.h"
#include "../../../pathfinder/lattice/LatticeSetup.h"
#include "../../../pathfinder/search/ConfigurationSpace.h"
#include "../../../pathfinder/coordtensor/debug_util.h"
#include "../../../pathfinder/modules/Metamodule.h"
#include <boost/test/tools/interface.hpp>

// set --log_level=all to see boost output

struct TestFixture {
    std::string fileS;
    std::string fileF;

    TestFixture() {
        fileS = "../docs/examples/moves/move_zigzag/zigzag_initial.json";
        fileF = "../docs/examples/moves/move_zigzag/zigzag_final.json";
    }
};

BOOST_FIXTURE_TEST_CASE(InitTest, TestFixture) {
    LatticeSetup::setupFromJson(fileS);
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves("../Moves");
}

BOOST_FIXTURE_TEST_CASE(TestNotIgnoreColorsAfterBFS, TestFixture) {
    bool ignoreColors = false;
    Lattice::setFlags(ignoreColors);
    Configuration start(Lattice::GetModuleInfo());
    Configuration end = LatticeSetup::setupFinalFromJson(fileF);
    auto path = ConfigurationSpace::BFS(&start, &end);
    for (auto config : path) {
        Lattice::UpdateFromModuleInfo(config->GetModData());
    }
    // BOOST_CHECK_NE(Lattice::colorTensor.Order(), 0);
    // BOOST_CHECK_NE(Lattice::colorTensor.AxisSize(), 0);
    Isometry::CleanupTransforms();
}

BOOST_FIXTURE_TEST_CASE(TestIgnoreColorsAfterBFS, TestFixture) {
    bool ignoreColors = true;
    Lattice::setFlags(ignoreColors);
    Configuration start(Lattice::GetModuleInfo());
    Configuration end = LatticeSetup::setupFinalFromJson(fileF);
    auto path = ConfigurationSpace::BFS(&start, &end);
    for (auto config : path) {
        Lattice::UpdateFromModuleInfo(config->GetModData());
    }
    // BOOST_CHECK_EQUAL(Lattice::colorTensor.Order(), 0);
    // BOOST_CHECK_EQUAL(Lattice::colorTensor.AxisSize(), 0);
    Isometry::CleanupTransforms();
}
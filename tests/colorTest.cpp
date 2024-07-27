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
#include "LatticeSetup.h"
#include "Scenario.h"
#include "MoveManager.h"
#include "Lattice.h"
#include "ConfigurationSpace.h"
#include "debug_util.h"
#include "MetaModule.h"

// set --log_level=all to see boost output

struct TestFixture {
    std::string fileS;
    std::string fileF;

    TestFixture() {
        fileS = "docs/examples/basic_3d_initial.json";
        fileF = "docs/examples/basic_3d_final.json";
    }
};

BOOST_FIXTURE_TEST_CASE(InitTest, TestFixture) {
    LatticeSetup::setupFromJson(fileS);
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves();
}

BOOST_FIXTURE_TEST_CASE(TestNotIgnoreColorsAfterBFS, TestFixture) {
    bool ignoreColors = false;
    Lattice::setFlags(ignoreColors);
    Configuration start(Lattice::stateTensor, Lattice::colorTensor);
    Configuration end = LatticeSetup::setupFinalFromJson(fileF);
    auto path = ConfigurationSpace::BFS(&start, &end);
    for (auto config : path) {
        Lattice::UpdateFromState(config->GetState(), config->GetColors());
    }
    BOOST_CHECK_NE(Lattice::colorTensor.Order(), 0);
    BOOST_CHECK_NE(Lattice::colorTensor.AxisSize(), 0);
    Isometry::CleanupTransforms();
}

BOOST_FIXTURE_TEST_CASE(TestIgnoreColorsAfterBFS, TestFixture) {
    bool ignoreColors = true;
    Lattice::setFlags(ignoreColors);
    Configuration start(Lattice::stateTensor, Lattice::colorTensor);
    Configuration end = LatticeSetup::setupFinalFromJson(fileF);
    auto path = ConfigurationSpace::BFS(&start, &end);
    for (auto config : path) {
        Lattice::UpdateFromState(config->GetState(), config->GetColors());
    }
    BOOST_CHECK_EQUAL(Lattice::colorTensor.Order(), 0);
    BOOST_CHECK_EQUAL(Lattice::colorTensor.AxisSize(), 0);
    Isometry::CleanupTransforms();
}
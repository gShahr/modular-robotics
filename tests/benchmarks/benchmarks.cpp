#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
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

int main() {
    ankerl::nanobench::Bench bench;
    std::string fileStart = "docs/examples/basic_3d_initial.json";
    std::string fileEnd = "docs/examples/basic_3d_final.json";
    bool ignoreColors = false;

    Lattice::setFlags(ignoreColors);
    LatticeSetup::setupFromJson(fileStart);
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves();
    Configuration start(Lattice::stateTensor, Lattice::colorTensor);
    Configuration end = LatticeSetup::setupFinalFromJson(fileEnd);

    // Benchmark the BFS function
    bench.run("BFS Benchmark", [&]() {
        auto path = ConfigurationSpace::BFS(&start, &end);
        ankerl::nanobench::doNotOptimizeAway(path);
    });
    return 0;
}
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
    LatticeSetup::setupFromJson("docs/examples/move_line_upside_down_initial.json");
    std::cout << Lattice::ToString();

    // Set up moves
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves();

    // BFS
    std::cout << "BFS Testing:\n";
    Configuration start(Lattice::stateTensor, Lattice::colorTensor);
    Configuration end = LatticeSetup::setupFinalFromJson("docs/examples/move_line_upside_down_final.json");
    auto path = ConfigurationSpace::BFS(&start, &end);
    std::cout << "Path:\n";
    for (auto config : path) {
        Lattice::UpdateFromState(config->GetState(), config->GetColors());
        std::cout << Lattice::ToString();
    }
    std::string exportFolder = "Visualization/Scenarios/";
    Scenario::exportToScen(path, exportFolder + "move_line_upside_down.scen");

    Isometry::CleanupTransforms();
    return 0;
}
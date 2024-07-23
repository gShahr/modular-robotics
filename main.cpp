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
    // // Set up Lattice
    // LatticeSetup::setupFromJson("docs/examples/move_line_initial.json");
    // std::cout << Lattice::ToString();

    // // Set up moves
    // MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    // MoveManager::RegisterAllMoves();

    // // BFS
    // std::cout << "BFS Testing:\n";
    // Configuration start(Lattice::stateTensor);
    // CoordTensor<bool> desiredState = LatticeSetup::setupFinal("docs/examples/move_line_final.txt");
    // Configuration end(desiredState);
    // auto path = ConfigurationSpace::BFS(&start, &end);
    // std::cout << "Path:\n";
    // for (auto config : path) {
    //     Lattice::UpdateFromState(config->GetState());
    //     std::cout << Lattice::ToString();
    // }
    // std::string exportFolder = "Visualization/Scenarios/";
    // Scenario::exportToScen(path, exportFolder + "test.scen");

    // // Cleanup
    // MoveManager::CleanMoves();
    MetaModuleManager metaModuleManager;
    MetaModule metamodule("tests/metamodule/metamodule_1.txt");
    MetaModuleManager::GenerateFrom(&metamodule);
    std::cout << "MetaModules Generated: " << MetaModuleManager::metamodules.size() << std::endl;
    LatticeSetup::setUpMetamodule(MetaModuleManager::metamodules[2]);
    std::string exportFolder = "Visualization/Scenarios/";
    Scenario::exportToScen(Lattice::stateTensor, exportFolder + "metamodule.scen");
    return 0;
}


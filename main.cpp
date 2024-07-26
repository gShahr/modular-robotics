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

int main(int argc, char* argv[]) {
    bool ignoreColors = false;
    std::string filename = "docs/examples/basic_3d_initial.json";

    // Define the long options
    static struct option long_options[] = {
        {"ignore-colors", no_argument, 0, 'i'},
        {"file", no_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "if:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'i':
                ignoreColors = true;
                break;
            case 'f':
                filename = optarg;
                break;
            case '?':
                break;
            default:
                abort();
        }
    }
    // if (filename.empty()) {
    //     std::cerr << "Filename is required. Use --file <filename>." << std::endl;
    //     return 1;
    // }
    // Set up Lattice
    Lattice::setFlags(ignoreColors);
    LatticeSetup::setupFromJson(filename);
    std::cout << Lattice::ToString();
    // Set up moves
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves();
    // BFS
    std::cout << "BFS Testing:\n";
    Configuration start(Lattice::stateTensor, Lattice::colorTensor);
    Configuration end = LatticeSetup::setupFinalFromJson("docs/examples/basic_3d_final.json");
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
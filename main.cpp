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

#define GENERATE_FINAL_STATE true

int main(int argc, char* argv[]) {
    bool ignoreColors = false;
    std::string initialFile = "docs/examples/move_nofinal_initial.json";
    std::string finalFile = "docs/examples/INVALID.json";
    std::string exportFile = initialFile.substr(0, initialFile.find_last_of('.')) + ".scen";

    // Define the long options
    static struct option long_options[] = {
        {"ignore-colors", no_argument, 0, 'i'},
        {"initial-file", required_argument, 0, 'I'},
        {"final-file", required_argument, 0, 'F'},
        {"export-file", required_argument, 0, 'e'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "iI:F:e:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'i':
                ignoreColors = true;
                break;
            case 'I':
                initialFile = optarg;
                break;
            case 'F':
                finalFile = optarg;
                break;
            case 'e':
                exportFile = optarg;
                break;
            case '?':
                break;
            default:
                abort();
        }
    }

    // Set up Lattice
    Lattice::setFlags(ignoreColors);
    LatticeSetup::setupFromJson(initialFile);
    std::cout << Lattice::ToString();
    
    // Set up moves
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves();
    
    // BFS
    std::cout << "BFS Testing:\n";
    Configuration start(Lattice::GetModuleInfo());
#if GENERATE_FINAL_STATE
    Configuration end = ConfigurationSpace::GenerateRandomFinal();
#else
    Configuration end = LatticeSetup::setupFinalFromJson(finalFile);
#endif
    std::vector<Configuration*> path;
    try {
        auto timeBegin = std::chrono::high_resolution_clock::now();
        path = ConfigurationSpace::AStar(&start, &end);
        auto timeEnd = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin);
        std::cout << "Search completed in " << duration.count() << " ms" << std::endl;
    } catch(BFSExcept& bfsExcept) {
        std::cerr << bfsExcept.what() << std::endl;
    }
    
    std::cout << "Path:\n";
    for (auto config : path) {
        Lattice::UpdateFromModuleInfo(config->GetModData());
        std::cout << Lattice::ToString();
    }
    
    Scenario::exportToScen(path, exportFile);
    Isometry::CleanupTransforms();
    return 0;
}
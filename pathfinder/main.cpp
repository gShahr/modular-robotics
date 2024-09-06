#include <chrono>
#include <iostream>
#include <getopt.h>
#include <set>
#include <string>
#include "moves/MoveManager.h"
#include "search/ConfigurationSpace.h"
#include "coordtensor/debug_util.h"
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <queue>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include "modules/Metamodule.h"
#include "lattice/LatticeSetup.h"
#include "moves/Scenario.h"
#include "search/SearchAnalysis.h"

#define GENERATE_FINAL_STATE false

int main(int argc, char* argv[]) {
    bool ignoreColors = false;
    std::string initialFile = "../docs/examples/moves/move_zigzag/zigzag_initial.json";
    std::string finalFile = "../docs/examples/moves/move_zigzag/zigzag_final.json";
    std::string exportFile = initialFile.substr(0, initialFile.find_last_of('.')) + ".scen";
    std::string analysisFile = initialFile.substr(0, initialFile.find_last_of('.')) + "_analysis.json";
    std::size_t trimPos;
    if ((trimPos = exportFile.find("_initial")) != std::string::npos) {
        exportFile.erase(trimPos, 8);
    }
    if ((trimPos = analysisFile.find("_initial")) != std::string::npos) {
        analysisFile.erase(trimPos, 8);
    }

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
    MoveManager::RegisterAllMoves("../Moves");
    
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
        const auto timeBegin = std::chrono::high_resolution_clock::now();
        path = ConfigurationSpace::BFS(&start, &end);
        const auto timeEnd = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin);
        std::cout << "Search completed in " << duration.count() << " ms" << std::endl;
#if CONFIG_OUTPUT_JSON
        SearchAnalysis::ExportData(analysisFile);
#endif
    } catch(BFSExcept& bfsExcept) {
        std::cerr << bfsExcept.what() << std::endl;
    }
    
    std::cout << "Path:\n";
    for (const auto config : path) {
        Lattice::UpdateFromModuleInfo(config->GetModData());
        std::cout << Lattice::ToString();
    }
    
    Scenario::exportToScen(path, exportFile);
    Isometry::CleanupTransforms();
    return 0;
}
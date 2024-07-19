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

namespace Scenario {
    void exportStateTensorToJson(int id, const CoordTensor<bool>& stateTensor, const std::string& filename) {
        int indentSize = 4;
        nlohmann::json jsonOutput;
        for (int i = 0; i < stateTensor.GetArrayInternal().size(); i++) {
            jsonOutput["configurations"][id]["state"].push_back(stateTensor.GetElementDirect(i));
        }
        std::ofstream file(filename);
        if (file.is_open()) {
            file << jsonOutput.dump(indentSize);
            file.close();
        }
    }

    void exportConfigurationSpaceToJson(const std::vector<Configuration*>& path, const std::string& filename) {
        for (int i = 0; i < path.size(); i++) {
            exportStateTensorToJson(i, path[i]->GetState(), filename);
        }
    }

    void exportToScen(Lattice& lattice, const std::vector<Configuration*>& path, const std::string& filename) {
        // File setup
        std::ofstream file(filename);
        // Group Definitions
        file << "0, 244, 244, 0, 95\n";
        file << "1, 255, 255, 255, 85\n\n";
        // Module Definitions
        auto idLen = std::to_string(ModuleIdManager::Modules().size()).size();
        boost::format padding("%%0%dd, %s");
        boost::format modDef((padding % idLen %  "%d, %d, %d, %d").str());
        lattice = path[0]->GetState();
        for (int id = 0; id < ModuleIdManager::Modules().size(); id++) {
            auto& mod = ModuleIdManager::Modules()[id];
            modDef % id % (mod.moduleStatic ? 1 : 0) % mod.coords[0] % mod.coords[1] % (mod.coords.size() > 2 ? mod.coords[2] : 0);
            file << modDef.str() << std::endl;
        }
        // Move Definitions
        file << std::endl;
        for (int i = 1; i < path.size(); i++) {
            auto movePair = MoveManager::FindMoveToState(lattice, path[i]->GetState());
            if (movePair.second == nullptr) {
                std::cout << "Failed to generate scenario file, no move to next state found.\n";
                file.close();
                return;
            }
            auto modToMove = movePair.first;
            for (const auto& anim : movePair.second->AnimSequence()) {
                modDef % modToMove->id % anim.first % anim.second[0] % anim.second[1] % anim.second[2];
                file << modDef.str() << std::endl;
            }
            //modDef % modToMove->id % lattice.coordTensor[modToMove->coords + move->AnchorOffset()] % move->MoveOffset()[0] % move->MoveOffset()[1] % (move->MoveOffset().size() > 2 ? move->MoveOffset()[2] : 0);
            //file << modDef.str() << std::endl;
            lattice.MoveModule(*modToMove, movePair.second->MoveOffset());
        }
        // File cleanup
        file.close();
    }
};

int main() {
    int order = 2;
    int axisSize = 9;
    std::string folder = "docs/examples/";
    Lattice lattice(order, axisSize);
    MoveManager::InitMoveManager(order, axisSize);
    LatticeSetup::setupFromJson(lattice, folder + "move_line_initial.json");
    std::cout << lattice;
    MoveManager::RegisterAllMoves();
    std::cout << "BFS Testing:\n";
    std::cout << "Original:    Desired:\n" <<
                 "  ----         --##\n" <<
                 "  -#--         ---#\n" <<
                 "  -##-         ----\n" <<
                 "  ----         ----\n";
    Configuration start(lattice.stateTensor);
    CoordTensor<bool> desiredState = LatticeSetup::setupFinal(order, axisSize, lattice, folder + "move_line_final.txt");
    Configuration end(desiredState);
    auto path = ConfigurationSpace::BFS(&start, &end, lattice);
    std::cout << "Path:\n";
    for (auto config : path) {
        lattice = config->GetState();
        std::cout << lattice;
    }
    Scenario::exportToScen(lattice, path, folder + "move_line_scen.scen");
    MoveManager::CleanMoves();
    return 0;
}
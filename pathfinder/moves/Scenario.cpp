#include "Scenario.h"
#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include "../modules/ModuleManager.h"
#include "MoveManager.h"
#include "../coordtensor/CoordTensor.h"
#include "../lattice/Lattice.h"
#include "../utility/color_util.h"

std::string Scenario::TryGetScenName(const std::string& initialFile) {
    std::ifstream stateFile(initialFile);
    nlohmann::json stateJson = nlohmann::json::parse(stateFile);
    if (stateJson.contains("name")) {
        return stateJson["name"];
    }
    std::string name = std::filesystem::path(initialFile).filename().stem();
    if (std::size_t trimPos = name.find("_initial"); trimPos != std::string::npos) {
        name.erase(trimPos, 8);
    }
    return name;
}

std::string Scenario::TryGetScenDesc(const std::string &initialFile) {
    std::ifstream stateFile(initialFile);
    nlohmann::json stateJson = nlohmann::json::parse(stateFile);
    if (stateJson.contains("description")) {
        return stateJson["description"];
    }
    return "Scenario file generated by pathfinder.";
}


void Scenario::exportToScen(const std::vector<Configuration *> &path, const ScenInfo &scenInfo) {
    if (path.empty()) {
        std::cerr << "Tried to export empty path, no good!" << std::endl;
        return;
    }
    std::ofstream file(scenInfo.exportFile);
    file << scenInfo.scenName << std::endl << scenInfo.scenDesc << std::endl;
#if LATTICE_RD_EDGECHECK
    file << "RHOMBIC_DODECAHEDRON\n\n";
#else
    file << "CUBE\n\n";
#endif
    if (Lattice::ignoreProperties) {
        file << "0, 244, 244, 0, 95\n";
        file << "1, 255, 255, 255, 85\n\n";
    } else {
        ModuleProperties::CallFunction("Palette");
        for (auto color: ResultHolder<std::unordered_set<int>>()) {
            Colors::ColorsRGB rgb(color);
            file << color << ", " << rgb.red << ", " << rgb.green << ", " << rgb.blue << ", 85\n";
        }
        file << "\n";
    }
    auto idLen = std::to_string(ModuleIdManager::Modules().size()).size();
    boost::format padding("%%0%dd, %s");
    boost::format modDef((padding % idLen % "%d, %d, %d, %d").str());
    Lattice::UpdateFromModuleInfo(path[0]->GetModData());
    for (size_t id = 0; id < ModuleIdManager::Modules().size(); id++) {
        auto &mod = ModuleIdManager::Modules()[id];
        if (Lattice::ignoreProperties) {
            modDef % id % (mod.moduleStatic ? 1 : 0) % mod.coords[0] % mod.coords[1] % (mod.coords.size() > 2
                    ? mod.coords[2]
                    : 0);
        } else {
            (mod.properties.Find(COLOR_PROP_NAME))->CallFunction("GetColorInt");
            modDef % id % ResultHolder<int>() % mod.
                    coords[0] % mod.coords[1] % (mod.coords.size() > 2 ? mod.coords[2] : 0);
        }
        file << modDef.str() << std::endl;
    }
    file << std::endl;
    for (size_t i = 1; i < path.size(); i++) {
        auto [movingModule, move] = MoveManager::FindMoveToState(path[i]->GetModData());
        if (move == nullptr) {
            std::cout << "Failed to generate scenario file, no move to next state found.\n";
            file.close();
            return;
        }
        auto modToMove = movingModule;
        for (const auto &[type, offset]: move->AnimSequence()) {
            modDef % modToMove->id % type % offset[0] % offset[1] % offset[2];
            file << modDef.str() << std::endl;
        }
        file << std::endl;
        Lattice::MoveModule(*modToMove, move->MoveOffset());
    }
    file.close();
}

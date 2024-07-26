#include "Scenario.h"
#include <iostream>
#include <boost/format.hpp>
#include "ModuleManager.h"
#include "MoveManager.h"
#include "Colors.h"

namespace ColorConverter {
    struct RGB {
        int red;
        int green;
        int blue;

        RGB(int r, int g, int b) : red(r), green(g), blue(b) {}

        explicit RGB(int rgbInt) {
            red = rgbInt & 0xFF0000;
            green = rgbInt & 0x00FF00;
            blue = rgbInt & 0x0000FF;
        }
    };

    std::map<std::string, RGB> colorToRGB = {
        {"red", {255, 0, 0}},
        {"green", {0, 255, 0}},
        {"blue", {0, 0, 255}},
        {"cyan", {0, 255, 255}},
        {"pink", {255, 192, 203}},
        {"orange", {255, 165, 0}},
        {"purple", {128, 0, 128}},
        {"yellow", {255, 255, 0}},
        {"brown", {165, 42, 42}},
        {"black", {0, 0, 0}},
        {"white", {255, 255, 255}},
        {"gray", {128, 128, 128}},
        {"lightgray", {211, 211, 211}},
        {"darkgray", {169, 169, 169}},
        {"magenta", {255, 0, 255}}
    };

    RGB convertColorNameToRGB(const std::string& colorName) {
        auto it = colorToRGB.find(colorName);
        if (it != colorToRGB.end()) {
            return it->second;
        } else {
            return RGB(0);
        }
    }

}

namespace Scenario {
    void exportStateTensorToJson(int id, const CoordTensor<bool>& stateTensor, const std::string& filename) {
        int indentSize = 4;
        nlohmann::json jsonOutput;
        for (size_t i = 0; i < stateTensor.GetArrayInternal().size(); i++) {
            jsonOutput["configurations"][id]["state"].push_back(stateTensor.GetElementDirect(i));
        }
        std::ofstream file(filename);
        if (file.is_open()) {
            file << jsonOutput.dump(indentSize);
            file.close();
        }
    }

    void exportConfigurationSpaceToJson(const std::vector<Configuration*>& path, const std::string& filename) {
        for (size_t i = 0; i < path.size(); i++) {
            exportStateTensorToJson(i, path[i]->GetState(), filename);
        }
    }

    void exportToScen(const std::vector<Configuration*>& path, const std::string& filename) {
        if (path.empty()) {
            std::cerr << "Tried to export empty path, no good!" << std::endl;
            return;
        }
        std::ofstream file(filename);
        if (Lattice::ignoreColors) {
            file << "0, 244, 244, 0, 95\n";
            file << "1, 255, 255, 255, 85\n\n";
        } else {
            for (auto color : Lattice::colorTensor.GetArrayInternal()) {
                ColorConverter::RGB rgb(color);
                file << color << ", " << rgb.red << ", " << rgb.green << ", " << rgb.blue << ", 85\n";
            }
            file << "\n";
        }
        auto idLen = std::to_string(ModuleIdManager::Modules().size()).size();
        boost::format padding("%%0%dd, %s");
        boost::format modDef((padding % idLen %  "%d, %d, %d, %d").str());
        Lattice::UpdateFromState(path[0]->GetState(), path[0]->GetColors());
        for (size_t id = 0; id < ModuleIdManager::Modules().size(); id++) {
            auto& mod = ModuleIdManager::Modules()[id];
            if (Lattice::ignoreColors) {
                modDef % id % (mod.moduleStatic ? 1 : 0) % mod.coords[0] % mod.coords[1] % (mod.coords.size() > 2 ? mod.coords[2] : 0);
            } else {
                modDef % id % Lattice::colorTensor[mod.coords] % mod.coords[0] % mod.coords[1] % (mod.coords.size() > 2 ? mod.coords[2] : 0);
            }
            file << modDef.str() << std::endl;
        }
        file << std::endl;
        for (size_t i = 1; i < path.size(); i++) {
            auto movePair = MoveManager::FindMoveToState(path[i]->GetState());
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
            file << std::endl;
            Lattice::MoveModule(*modToMove, movePair.second->MoveOffset());
        }
        file.close();
    }

    void exportToScen(const CoordTensor<bool>& state, const CoordTensor<int>& colors, const std::string& filename) {
        std::ofstream file(filename);
        file << "0, 244, 244, 0, 95\n";
        file << "1, 255, 255, 255, 85\n\n";
        auto idLen = std::to_string(ModuleIdManager::Modules().size()).size();
        boost::format padding("%%0%dd, %s");
        boost::format modDef((padding % idLen %  "%d, %d, %d, %d").str());
        Lattice::UpdateFromState(state, colors);
        for (size_t id = 0; id < ModuleIdManager::Modules().size(); id++) {
            auto& mod = ModuleIdManager::Modules()[id];
            modDef % id % (mod.moduleStatic ? 1 : 0) % mod.coords[0] % mod.coords[1] % (mod.coords.size() > 2 ? mod.coords[2] : 0);
            file << modDef.str() << std::endl;
        }
        file << std::endl;
        file.close();
    }
}
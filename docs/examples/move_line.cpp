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

/**
 * @brief Sets up a lattice structure from a JSON file.
 * 
 * This function reads a JSON file specified by `filename` and uses its contents
 * to configure a `Lattice` object. The JSON file is expected to contain a list
 * of modules, each with a position and a static flag. The position is adjusted
 * by adding the `ORIGIN` offset to each coordinate, allowing for repositioning
 * of the entire structure within the lattice's coordinate system.
 * 
 * @param ORIGIN An integer offset added to each coordinate of the module's position
 * to translate the module within the lattice.
 * @param lattice A reference to the Lattice object that will be populated with
 * modules based on the JSON file.
 * @param filename The path to the JSON file containing the configuration for the
 * lattice. The file should be structured to include a "modules" array, with each
 * object in the array having a "position" array of integers and a "static" boolean.
 * 
 * @note If the file cannot be opened, an error message is printed to `std::cerr`.
 * The function modifies the `lattice` in place by adding modules according to the
 * JSON file's specifications. The "static" flag for each module is directly passed
 * to the `AddModule` method of the `Lattice` object.
 */
void setupFromJson(Lattice& lattice, const std::string& filename);
void setupFromJson(Lattice& lattice, const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return;
    }
    nlohmann::json j;
    file >> j;
    for (const auto& module : j["modules"]) {
        std::vector<int> position = module["position"];
        std::transform(position.begin(), position.end(), position.begin(),
                    [](int coord) { return coord; });
        std::valarray<int> coords(position.data(), position.size());
        lattice.AddModule(coords, module["static"]);
    }
    lattice.BuildMovableModules();
}

/**
 * @brief Reads module configuration from a file and initializes the lattice.
 * 
 * This function opens a file specified by `filename` to configure a `lattice`
 * object and generate a visual representation of the lattice in a 2D character
 * vector `image`. The file's content dictates the placement and type of modules
 * within the lattice, where each character ('1', '0', '@') in a line corresponds
 * to a module's state or type: '1' for a standard module, '0' for an empty space,
 * and '@' for a static module. The `ORIGIN` parameter determines the starting
 * coordinates for module placement, aligning the file's content with the lattice's
 * coordinate system.
 * 
 * @param ORIGIN The base coordinate offset for x and y axes, setting the initial
 * position for parsing.
 * @param lattice A reference to the Lattice object that will be populated with
 * modules based on the file.
 * @param filename The path to the input file containing the configuration for the
 * lattice and its visual representation.
 * 
 * @note The function modifies the `lattice` in place and prints the visual
 * representation to standard output. If the file cannot be opened, an error message
 * is output to `std::cerr`. The function also identifies and marks articulation
 * points within the lattice after all modules have been added.
 */
void setupInitial(Lattice& lattice, const std::string& filename);
void setupInitial(Lattice& lattice, const std::string& filename) {
    std::vector<std::vector<char>> image;
    int x = 0;
    int y = 0;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        for (char c: line) {
            if (c == '1') {
                std::valarray<int> coords = {x, y};
                lattice.AddModule(coords);
            } else if (c == '0') {
                // Do nothing
            } else if (c == '@') {
                std::valarray<int> coords = {x, y};
                lattice.AddModule(coords, true);
            }
            x++;
        }
        x = 0;
        y++;
    }
    file.close();
    lattice.BuildMovableModules();
}

/**
 * @brief Reads a configuration from a file and sets up the desired state in a CoordTensor.
 * 
 * This function opens a file specified by `filename` and reads its contents line by line
 * to configure a `CoordTensor<bool>` representing the desired state. Each character in the
 * file is examined, and if it is '1', the corresponding position in the `CoordTensor` is
 * set to `true`, indicating a desired active state at that position. The function uses
 * `ORIGIN` to reset the `x` coordinate at the start of each new line and after the file
 * is fully processed. The `order` and `axisSize` parameters are used to initialize the
 * `CoordTensor` dimensions and initial state.
 * 
 * @param order The order (dimensionality) of the CoordTensor.
 * @param axisSize The size of each axis in the CoordTensor.
 * @param ORIGIN The starting point for the x and y coordinates, used for resetting.
 * @param lattice A reference to a Lattice object, not used in the current implementation but
 *                included for potential future use or extension.
 * @param filename The name of the file from which to read the configuration.
 * 
 * @return A CoordTensor<bool> representing the desired state as configured by the file.
 * 
 * @note If the file cannot be opened, an error message is printed to `std::cerr` and the
 *       function returns early with an uninitialized CoordTensor. This behavior might need
 *       handling by the caller to avoid undefined behavior.
 */
CoordTensor<bool> setupFinal(int order, int axisSize, Lattice& lattice, const std::string& filename);
CoordTensor<bool> setupFinal(int order, int axisSize, Lattice& lattice, const std::string& filename) {
    int x = 0;
    int y = 0;
    CoordTensor<bool> desiredState(order, axisSize, false);
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return desiredState;
    }
    std::string line;
    while (std::getline(file, line)) {
        for (char c: line) {
            if (c == '1') {
                std::valarray<int> coords = {x, y};
                desiredState[coords] = true;
            }
            x++;
        }
        x = 0;
        y++;
    }
    file.close();
    return desiredState;
}

int main() {
    int order = 2;
    int axisSize = 9;
    std::string folder = "docs/examples/";
    Lattice lattice(order, axisSize);
    MoveManager::InitMoveManager(order, axisSize);
    setupFromJson(lattice, folder + "move_line_initial.json");
    std::cout << lattice;
    MoveManager::RegisterAllMoves();
    std::cout << "BFS Testing:\n";
    std::cout << "Original:    Desired:\n" <<
                 "  ----         --##\n" <<
                 "  -#--         ---#\n" <<
                 "  -##-         ----\n" <<
                 "  ----         ----\n";
    Configuration start(lattice.stateTensor);
    CoordTensor<bool> desiredState = setupFinal(order, axisSize, lattice, folder + "move_line_final.txt");
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
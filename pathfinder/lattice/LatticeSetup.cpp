#include "LatticeSetup.h"
#include "../search/ConfigurationSpace.h"
#include "../modules/Metamodule.h"
#include "../modules/Colors.h"
#include <set>

namespace LatticeSetup {
    void setupFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            return;
        }
        nlohmann::json j;
        file >> j;
        Lattice::InitLattice(j["order"], j["axisSize"]);
        std::set<int> colors;
        for (const auto& module : j["modules"]) {
            std::vector<int> position = module["position"];
            std::transform(position.begin(), position.end(), position.begin(),
                        [](int coord) { return coord; });
            std::valarray<int> coords(position.data(), position.size());
            if (!Lattice::ignoreProperties && module.contains("properties")) {
                ModuleIdManager::RegisterModule(coords, module["static"], module["properties"]);
                //colors.insert(Colors::colorToInt[module["color"]]);
            } else {
                ModuleIdManager::RegisterModule(coords, module["static"]);
            }
        }
        // Register static modules after non-static modules
        ModuleIdManager::DeferredRegistration();
        if (ColorProperty::Palette().size() <= 1) {
            //Lattice::colorTensor = CoordTensor<int>(0, 0, 0);
            //TODO: add property stuff here
            Lattice::ignoreProperties = true;
        }
        for (const auto& mod : ModuleIdManager::Modules()) {
            Lattice::AddModule(mod);
        }
        Lattice::BuildMovableModules();
    }

    Configuration setupFinalFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            throw std::ios_base::failure("Unable to open file " + filename + "\n");
        }
        nlohmann::json j;
        file >> j;
        //CoordTensor<bool> desiredState(Lattice::Order(), Lattice::AxisSize(), false);
        //CoordTensor<int> desiredColors(Lattice::Order(), Lattice::AxisSize(), -1);
        std::set<ModuleBasic> desiredState;
        for (const auto& module : j["modules"]) {
            if (module["static"] == true) continue;
            std::vector<int> position = module["position"];
            std::transform(position.begin(), position.end(), position.begin(),
                        [](int coord) { return coord; });
            std::valarray<int> coords(position.data(), position.size());
            //desiredState[coords] = true;
            ModuleProperties props;
            if (!Lattice::ignoreProperties && module.contains("properties")) {
                props.InitProperties(module["properties"]);
                //desiredColors[coords] = Colors::colorToInt[module["color"]];
            }
            desiredState.insert({coords, props});
        }
        //return {desiredState, desiredColors};
        //TODO: add property stuff here
        return Configuration(desiredState);
    }

    [[deprecated("Should use setupFromJson instead")]]
    void setupInitial(const std::string& filename, int order, int axisSize) {
        Lattice::InitLattice(order, axisSize);
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
            for (char c : line) {
                if (c == '1') {
                    std::valarray<int> coords = {x, y};
                    ModuleIdManager::RegisterModule(coords, false);
                } else if (c == '@') {
                    std::valarray<int> coords = {x, y};
                    ModuleIdManager::RegisterModule(coords, true);
                }
                x++;
            }
            x = 0;
            y++;
        }
        ModuleIdManager::DeferredRegistration();
        for (const auto& mod : ModuleIdManager::Modules()) {
            Lattice::AddModule(mod);
        }
        Lattice::BuildMovableModules();
    }

    [[deprecated("Should use setupFinalFromJson instead")]]
    Configuration setupFinal(const std::string& filename) {
        int x = 0;
        int y = 0;
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file " << filename << std::endl;
            throw std::ios_base::failure("Unable to open file " + filename + "\n");
        }
        CoordTensor<bool> desiredState(Lattice::Order(), Lattice::AxisSize(), false);
        CoordTensor<int> colors(Lattice::Order(), Lattice::AxisSize(), -1);
        std::string line;
        while (std::getline(file, line)) {
            for (char c : line) {
                if (c == '1') {
                    std::valarray<int> coords = {x, y};
                    desiredState[coords] = true;
                }
                x++;
            }
            x = 0;
            y++;
        }
        return Configuration({});
    }

    void setUpMetamodule(MetaModule* metamodule) {
        Lattice::InitLattice(metamodule->order, metamodule->size);
        for (const auto &coord: metamodule->coords) {
            ModuleIdManager::RegisterModule(coord.second, coord.first);
        }
        ModuleIdManager::DeferredRegistration();
        for (const auto& mod : ModuleIdManager::Modules()) {
            Lattice::AddModule(mod);
        }
    }

    void setUpTiling() {
        Lattice::InitLattice(MetaModuleManager::order, MetaModuleManager::axisSize);
        for (int i = 0; i < MetaModuleManager::axisSize / MetaModuleManager::metamodules[0]->size; i++) {
            for (int j = 0; j < MetaModuleManager::axisSize / MetaModuleManager::metamodules[0]->size; j++) {
                if ((i%2==0 && j&1) || (i&1 && j%2 == 0)) {
                    for (const auto &coord: MetaModuleManager::metamodules[5]->coords) {
                        std::valarray<int> newCoord = {MetaModuleManager::metamodules[5]->size * i, MetaModuleManager::metamodules[5]->size * j};
                        ModuleIdManager::RegisterModule(coord.second + newCoord, coord.first);
                    }
                } else {
                    for (const auto &coord: MetaModuleManager::metamodules[0]->coords) {
                        std::valarray<int> newCoord = {MetaModuleManager::metamodules[0]->size * i, MetaModuleManager::metamodules[0]->size * j};
                        ModuleIdManager::RegisterModule(coord.second + newCoord, coord.first);
                    }
                }
            }
        }
        ModuleIdManager::DeferredRegistration();
        for (const auto& mod : ModuleIdManager::Modules()) {
            Lattice::AddModule(mod);
        }
        // for (const auto &coord: MetaModuleManager::metamodules[0]->coords) {
        //     Lattice::AddModule(coord);
        // }
        // for (const auto &coord: MetaModuleManager::metamodules[5]->coords) {
        //     std::valarray<int> newCoord = {3, 0};
        //     Lattice::AddModule(coord + newCoord);
        // }
        // for (const auto &coord: MetaModuleManager::metamodules[0]->coords) {
        //     std::valarray<int> newCoord = {3, 3};
        //     Lattice::AddModule(coord + newCoord);
        // }
        // for (const auto &coord: MetaModuleManager::metamodules[5]->coords) {
        //     std::valarray<int> newCoord = {0, 3};
        //     Lattice::AddModule(coord + newCoord);
        // }
    }

    void setUpTilingFromJson(const std::string& metamoduleFile, const std::string& config) {
        // TODO Replace hard-coded values
        int metamoduleOrder = 2;
        int metamoduleSize = 3;
        int latticeOrder = metamoduleOrder;
        int latticeSize = 10;
        MetaModule metamodule(metamoduleFile, metamoduleOrder, metamoduleSize);
        std::ifstream file(config);
        if (!file) {
            std::cerr << "Unable to open file " << config << std::endl;
            return;
        }
        nlohmann::json j;
        file >> j;
        // Potentially move this to main function because it is static
        MetaModuleManager::InitMetaModuleManager(metamoduleOrder, metamoduleSize);
        MetaModuleManager::GenerateFrom(&metamodule);
        for (const auto& metamodule : j["metamodules"]) {
            std::valarray<int> position = metamodule["position"];
            std::string config = metamodule["config"];
            // TODO 
            // Pick the correct metamodule
            // Replace coord.first with something more descriptive
            MetaModule* currentMetamodule = MetaModuleManager::metamodules[0];
            for (const auto &coord: currentMetamodule->coords) {
                std::valarray<int> newCoord = coord.second + position * currentMetamodule->size;
                ModuleIdManager::RegisterModule(coord.second + newCoord, coord.first);
            }
        }
        Lattice::InitLattice(MetaModuleManager::order, MetaModuleManager::axisSize);
        ModuleIdManager::DeferredRegistration();
        for (const auto& mod : ModuleIdManager::Modules()) {
            Lattice::AddModule(mod);
        }
    }
}
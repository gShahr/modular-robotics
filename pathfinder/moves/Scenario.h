#ifndef SCENARIO_H
#define SCENARIO_H

#include <string>
#include <vector>
#include "../search/ConfigurationSpace.h"
#include "nlohmann/json.hpp"

namespace Scenario {
    struct ScenInfo {
        std::string exportFile;
        std::string scenName;
        std::string scenDesc;
    };

    void exportToScen(const std::vector<Configuration*>& path, const ScenInfo& scenInfo);
}

#endif
#ifndef SCENARIO_H
#define SCENARIO_H

#include <string>
#include <vector>
#include "../search/ConfigurationSpace.h"
#include "nlohmann/json.hpp"

namespace Scenario {
    //void exportStateTensorToJson(int id, const CoordTensor<bool>& stateTensor, const std::string& filename);
    //void exportConfigurationSpaceToJson(const std::vector<Configuration*>& path, const std::string& filename);
    void exportToScen(const std::vector<Configuration*>& path, const std::string& filename);
    //void exportToScen(const CoordTensor<bool>& state, const CoordTensor<int>& colors, const std::string& filename);
}

#endif
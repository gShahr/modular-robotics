#ifndef SCENARIO_H
#define SCENARIO_H

#include <string>
#include <vector>
#include "CoordTensor.h"
#include "ConfigurationSpace.h"
#include "Lattice.h"
#include "nlohmann/json.hpp"
#include <fstream>

namespace Scenario {
    void exportStateTensorToJson(int id, const CoordTensor<bool>& stateTensor, const std::string& filename);
    void exportConfigurationSpaceToJson(const std::vector<Configuration*>& path, const std::string& filename);
    void exportToScen(const std::vector<Configuration*>& path, const std::string& filename);
};

#endif
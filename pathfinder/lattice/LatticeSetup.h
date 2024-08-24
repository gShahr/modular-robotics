#ifndef LATTICESETUP_H
#define LATTICESETUP_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <valarray>
#include "Lattice.h"
#include "../coordtensor/CoordTensor.h"
#include "../search/ConfigurationSpace.h"
#include "../modules/Metamodule.h"
#include <nlohmann/json.hpp>

namespace LatticeSetup {
    void setupFromJson(const std::string& filename);

    Configuration setupFinalFromJson(const std::string& filename);

    void setupInitial(const std::string& filename);

    Configuration setupFinal(const std::string& filename);

    void setUpMetamodule(MetaModule* metamodule);
    
    void setUpTiling();

    void setUpTilingFromJson(const std::string& filename);
}

#endif
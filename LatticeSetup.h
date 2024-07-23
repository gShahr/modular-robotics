#ifndef LATTICESETUP_H
#define LATTICESETUP_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <valarray>
#include "Lattice.h"
#include "CoordTensor.h"
#include "Metamodule.h"
#include <nlohmann/json.hpp>

namespace LatticeSetup {
    void setupFromJson(const std::string& filename);

    void setupInitial(const std::string& filename);

    CoordTensor<bool> setupFinal(const std::string& filename);

    void setUpMetamodule(MetaModule* metamodule);
    
    void setUpTiling();
};

#endif
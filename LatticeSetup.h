#ifndef LATTICESETUP_H
#define LATTICESETUP_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <valarray>
#include "Lattice.h"
#include "CoordTensor.h"
#include <nlohmann/json.hpp>

namespace LatticeSetup {
    void setupFromJson(Lattice& lattice, const std::string& filename);

    void setupInitial(Lattice& lattice, const std::string& filename);

    CoordTensor<bool> setupFinal(int order, int axisSize, Lattice& lattice, const std::string& filename);
};

#endif
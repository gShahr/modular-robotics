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
#include "LatticeSetup.h"
#include "Scenario.h"

int main() {
    std::string folder = "examples/";
    MetaModule metaModule(folder + "2_by_2_metamodule.txt");
    metaModule.generateRotations();
    metaModule.generateReflections();
    metaModule.printConfigurations();
    return 0;
}
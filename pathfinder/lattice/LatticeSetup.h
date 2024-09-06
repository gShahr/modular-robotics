#ifndef LATTICESETUP_H
#define LATTICESETUP_H

#include <string>
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
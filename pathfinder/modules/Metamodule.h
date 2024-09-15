#ifndef METAMODULE_H
#define METAMODULE_H

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>
#include <valarray>
#include <nlohmann/json.hpp>
#include "../moves/Isometry.h"
#include "../coordtensor/CoordTensor.h"
#include "../properties/Colors.h"

class MetaModule final : public ITransformable {
private:
public:
    // <color, coords>
    std::vector<std::pair<int, std::valarray<int>>> coords;
    int order;
    int size;

    MetaModule(const std::string& filename, int order, int size);

    MetaModule* MakeCopy() const override;

    void readFromTxt2d(const std::string& filename);

    void readFromJson(const std::string& filename);

    void Rotate(int a, int b) override;

    void Reflect(int index) override;

    void printCoordsOnly() const;

    void printConfigurations() const;
};

class MetaModuleManager {
public:
    static std::vector<MetaModule*> metamodules;
    static int order;
    static int axisSize;

    static void InitMetaModuleManager(int _order, int _axisSize);

    static void GenerateFrom(MetaModule* metamodule);
};

#endif
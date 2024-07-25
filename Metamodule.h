#ifndef METAMODULE_H
#define METAMODULE_H

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>
#include <valarray>
#include <nlohmann/json.hpp>
#include "Isometry.h"

class MetaModule : public ITransformable {
private:
public:
    std::vector<std::valarray<int>> coords;
    int order;
    int size;

    MetaModule(const std::string& filename, int order, int size);

    MetaModule* MakeCopy() const override;

    void readFromTxt2d(const std::string& filename);

    void readFromJson(const std::string& filename);

    void Rotate(int a, int b) override;

    void Reflect(int index) override;

    void generateRotations();

    void generateReflections();

    void printCoordsOnly() const;

    void printConfigurations() const;
};

class MetaModuleManager {
public:
    static std::vector<MetaModule*> metamodules;
    static int order;
    static int axisSize;

    MetaModuleManager(int order, int axisSize);

    static void GenerateFrom(MetaModule* metamodule);
};

#endif
#ifndef METAMODULE_H
#define METAMODULE_H

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>
#include <valarray>
#include "Isometry.h"

class MetaModule : public ITransformable {
private:
public:
    std::vector<std::valarray<int>> coords;
    int axisSize;

    MetaModule(const std::string& filename);

    MetaModule* MakeCopy() const override;

    void Rotate(int index) override;

    void Reflect(int index) override;

    void generateRotations();

    void generateReflections();

    void printCoordsOnly() const;

    void printConfigurations() const;
};

class MetaModuleManager
{
    static std::vector<MetaModule*> metamodules;

    static void GenerateFrom(MetaModule* metamodule);
};

#endif
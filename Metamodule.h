#ifndef METAMODULE_H
#define METAMODULE_H

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>

class MetaModule {
private:
public:
    std::vector<std::pair<int, int>> coords;
    std::vector<std::vector<std::pair<int, int>>> metaModules;
    int order;
    int axisSize;

    MetaModule(const std::string& filename);
    void generateRotations();
    void generateReflections();
    void printCoordsOnly() const;
    void printConfigurations() const;
};

#endif
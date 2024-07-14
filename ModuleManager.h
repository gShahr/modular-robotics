#include <iostream>
#include <vector>
#include <valarray>

#ifndef MODULAR_ROBOTICS_MODULEMANAGER_H
#define MODULAR_ROBOTICS_MODULEMANAGER_H

// Class used to hold info about each module
class Module {
public:
    // Coordinate information
    std::valarray<int> coords;
    // Static module check
    bool moduleStatic = false;
    // Module ID
    int id;

    explicit Module(const std::valarray<int>& coords, bool isStatic = false);
};

// Class responsible for module ID assignment and providing a central place where modules are stored
class ModuleIdManager {
private:
    // ID to be assigned to next module during construction
    static int _nextId;
    // Vector holding all modules, indexed by module ID
    static std::vector<Module> _modules;

public:
    // Never instantiate ModuleIdManager
    ModuleIdManager() = delete;
    ModuleIdManager(const ModuleIdManager&) = delete;

    // Emplace newly created module into the vector
    static void RegisterModule(Module& module);

    // Get ID for assignment to newly created module
    [[nodiscard]]
    static int GetNextId();

    // Get vector of modules
    static std::vector<Module>& Modules();
};

// Stream insertion operator overloaded for easy printing of module info
std::ostream& operator<<(std::ostream& out, const Module& mod);

#endif //MODULAR_ROBOTICS_MODULEMANAGER_H

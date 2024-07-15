#include "ModuleManager.h"
#include "CoordTensor.h"

#ifndef MODULAR_ROBOTICS_LATTICE_H
#define MODULAR_ROBOTICS_LATTICE_H

class Lattice {
private:
    // Vector that holds the IDs of adjacent modules, indexed by ID
    std::vector<std::vector<int>> adjList;
    // Order of coordinate tensor / # of dimensions
    int order;
    // Length of every axis
    int axisSize;
    // Time variable for DFS
    int time = 0;
    // # of modules
    int moduleCount = 0;
    // Vector of movable modules
    std::vector<Module*> movableModules;

    // Clear adjacency list for module ID, and remove module ID from other lists
    void ClearAdjacencies(int moduleId);

public:
    // State tensor
    CoordTensor<bool> stateTensor;
    // Module tensor
    CoordTensor<int> coordTensor;
    // Coordinate info for articulation points / cut vertices
    std::vector<std::valarray<int>> articulationPoints;

    Lattice(int order, int axisSize);

    // Add a new module
    void AddModule(const std::valarray<int>& coords, bool isStatic = false);

    // Move a module
    void MoveModule(Module& mod, const std::valarray<int>& offset);

    // Adjacency Check
    void EdgeCheck(const Module& mod, bool bothWays = true);

    // Update adjacency lists for two adjacent modules
    void AddEdge(int modA, int modB);

    // Find articulation points / cut vertices using DFS
    void APUtil(int u, std::vector<bool>& visited, std::vector<bool>& ap, std::vector<int>& parent, std::vector<int>& low, std::vector<int>& disc);

    // Build / Rebuild movableModules vector
    void BuildMovableModules();

    // Get movable modules
    const std::vector<Module*>& MovableModules();

    // Comparison operator
    bool operator==(const Lattice& other);

    // Assign from state tensor
    Lattice& operator=(const CoordTensor<bool>& state);

    friend std::ostream& operator<<(std::ostream& out, Lattice& lattice);
};

std::ostream& operator<<(std::ostream& out, /*const*/ Lattice& lattice);

#endif //MODULAR_ROBOTICS_LATTICE_H

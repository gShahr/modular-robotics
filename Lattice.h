#include "ModuleManager.h"
#include "CoordTensor.h"

#ifndef MODULAR_ROBOTICS_LATTICE_H
#define MODULAR_ROBOTICS_LATTICE_H

// Verbosity Constants (Don't change these)
#define LAT_LOG_NONE 0
#define LAT_LOG_ADJ 1
#define LAT_LOG_CUT 2
#define LAT_LOG_ALL 3
/* Verbosity Configuration
 * NONE: No output from Lattice
 * ADJ: Output adjacency information
 * CUT: Output cut vertex information
 * ALL: Output both adjacency and cut vertex information
 */
#define LATTICE_VERBOSE LAT_LOG_NONE


class Lattice {
private:
    // Vector that holds the IDs of adjacent modules, indexed by ID
    static std::vector<std::vector<int>> adjList;
    // Order of coordinate tensor / # of dimensions
    static int order;
    // Length of every axis
    static int axisSize;
    // Time variable for DFS
    static int time;
    // # of modules
    static int moduleCount;
    // Vector of movable modules
    static std::vector<Module*> movableModules;

    // Clear adjacency list for module ID, and remove module ID from other lists
    static void ClearAdjacencies(int moduleId);

public:
    // State tensor
    static CoordTensor<bool> stateTensor;
    // Module tensor
    static CoordTensor<int> coordTensor;
    // Color tensor
    static CoordTensor<std::string> colorTensor;

    Lattice() = delete;
    Lattice(Lattice&) = delete;

    //Lattice(int order, int axisSize);
    static void InitLattice(int _order, int _axisSize);

    // Add a new module
    static void AddModule(const std::valarray<int>& coords, bool isStatic = false, const std::string& color = "");

    // Move a module
    static void MoveModule(Module& mod, const std::valarray<int>& offset);

    // Adjacency Check
    static void EdgeCheck(const Module& mod, bool bothWays = true);

    // Update adjacency lists for two adjacent modules
    static void AddEdge(int modA, int modB);

    // Find articulation points / cut vertices using DFS
    static void APUtil(int u, std::vector<bool>& visited, std::vector<bool>& ap, std::vector<int>& parent, std::vector<int>& low, std::vector<int>& disc);

    // Build / Rebuild movableModules vector
    static void BuildMovableModules();

    // Get movable modules
    static const std::vector<Module*>& MovableModules();

    // Assign from state tensor
    static void UpdateFromState(const CoordTensor<bool>& state, const CoordTensor<std::string>& colors);

    static int Order();

    static int AxisSize();

    static std::string ToString();

    friend class MoveManager;
};

#endif //MODULAR_ROBOTICS_LATTICE_H

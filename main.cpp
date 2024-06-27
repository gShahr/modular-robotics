#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <fstream>
#include <set>
#include <string>
// CoordTensor not used yet, but it's all set up, just need to refactor main.cpp
#include "CoordTensor.h"
#include "debug_util.h"
#include <boost/functional/hash.hpp>

class Module;

// Class responsible for module ID assignment and providing a central place where modules are stored
class ModuleIdManager {
private:
    // ID to be assigned to next module during construction
    static int _nextId;
    // Vector holding all modules, indexed by module ID
    static std::vector<Module> _modules;

public:
    // Emplace newly created module into the vector
    static void RegisterModule(Module& module) {
        _modules.emplace_back(module);
    }

    // Get ID for assignment to newly created module
    static int GetNextId() {
        return _nextId++;
    }

    // Get read access to vector of modules, indexed by ID
    static std::vector<Module>& Modules() {
        return _modules;
    }
};

int ModuleIdManager::_nextId = 0;
std::vector<Module> ModuleIdManager::_modules;

class Module {
public:
    // Coordinate information
    std::valarray<int> coords;
    // Module ID
    int id;

    explicit Module(const std::valarray<int>& coords) : coords(coords), id(ModuleIdManager::GetNextId()) { }
};

// Stream insertion operator overloaded for easy printing of module info
std::ostream& operator<<(std::ostream& out, const Module& mod) {
    out << "Module with ID " << mod.id << " at ";
    std::string sep = "(";
    for (auto coord : mod.coords) {
        out << sep << coord;
        sep = ", ";
    }
    out << ")";
    return out;
}

class Lattice {
private:
    // Vector that holds the IDs of adjacent modules, indexed by ID
    std::vector<std::vector<int>> adjlist;
    // Order of coordinate tensor / # of dimensions
    int order;
    // Length of every axis
    int axisSize;
    // Time variable for DFS
    int time;
    // # of modules
    int moduleCount;

    // Clear adjacency list for module id, and remove module id from other lists
    void ClearAdjacencies(int moduleId) {
        for (auto id : adjlist[moduleId]) {
            for (int i = 0; i < adjlist[id].size(); i++) {
                if (adjlist[id][i] == moduleId) {
                    adjlist[id].erase(adjlist[id].begin() + i);
                    break;
                }
            }
        }
        adjlist[moduleId].clear();
    }

public:
    std::vector<bool> grid;
    // CoordTensor, should eventually replace coordmat
    CoordTensor coordTensor;
    // Holds coordinate info for articulation points / cut vertices
    std::vector<std::valarray<int>> articulationPoints;

    Lattice(int order, int axisSize) : coordTensor(order, axisSize), order(order), axisSize(axisSize), time(0), moduleCount(0) {
        grid.resize(pow(axisSize, order), false);
    }

    // Add a new module
    void addModule(const std::valarray<int>& coords) {
        int index = 0;
        for (int i = 0; i < coords.size(); i++) {
             index += i * pow(axisSize, i) * coords[i];
        }
        grid[index] = true;
        // Create and register new module
        Module mod(coords);
        ModuleIdManager::RegisterModule(mod);
        // Insert module ID at given coordinates
        coordTensor[{coords}] = mod.id;
        // bothWays bool set to false due to how the lattice should be built
        edgeCheck(mod, false);
        moduleCount++;
        adjlist.resize(moduleCount + 1);
    }

    // Move
    void moveModule(Module& mod, const std::valarray<int>& offset) {
        ClearAdjacencies(mod.id);
        coordTensor.IdAt(mod.coords) = -1;
        mod.coords += offset;
        coordTensor.IdAt(mod.coords) = mod.id;
        edgeCheck(mod);
    }

    // New generalized edgeCheck
    void edgeCheck(const Module& mod, bool bothWays = true) {
        // copy module coordinates to adjCoords
        auto adjCoords = mod.coords;
        for (int i = 0; i < order; i++) {
            // Don't want to check index -1
            if (adjCoords[i] == 0) continue;
            adjCoords[i]--;
            if (coordTensor[adjCoords] >= 0) {
                DEBUG(mod << " Adjacent to " << ModuleIdManager::Modules()[coordTensor[adjCoords]] << std::endl);
                addEdge(mod.id, coordTensor[adjCoords]);
            }
            // Don't want to check both ways if it can be avoided, also don't want to check index beyond max value
            if (!bothWays || adjCoords[i] + 1 == axisSize) {
                adjCoords[i]++;
                continue;
            }
            adjCoords[i] += 2;
            if (coordTensor[adjCoords] >= 0) {
                DEBUG(mod << " Adjacent to " << ModuleIdManager::Modules()[coordTensor[adjCoords]] << std::endl);
                addEdge(mod.id, coordTensor[adjCoords]);
            }
            adjCoords[i]--;
        }
    }

    // Original edgeCheck
    void edgeCheck2D(const Module& mod) {
        if (coordTensor[{mod.coords[0] - 1, mod.coords[1]}] >= 0) {
            DEBUG("Module at " << mod.coords[0] << ", " << mod.coords[1] << " Adjacent to module at " << mod.coords[0] - 1 << ", " << mod.coords[1] << std::endl);
            addEdge(mod.id, coordTensor[{mod.coords[0] - 1, mod.coords[1]}]);
        }
        if (coordTensor[{mod.coords[0], mod.coords[1] - 1}] >= 0) {
            DEBUG("Module at " << mod.coords[0] << ", " << mod.coords[1] << " Adjacent to module at " << mod.coords[0] << ", " << mod.coords[1] - 1 << std::endl);
            addEdge(mod.id, coordTensor[{mod.coords[0], mod.coords[1] - 1}]);
        }
    }

    // Update adjacency lists for module IDs u and v
    void addEdge(int u, int v) {
        adjlist[u].push_back(v);
        adjlist[v].push_back(u);
    }

    // Find articulation points / cut vertices using DFS
    void APUtil(int u, std::vector<bool>& visited, std::vector<bool>& ap, std::vector<int>& parent, std::vector<int>& low, std::vector<int>& disc) {
        int children = 0;
        visited[u] = true;
        disc[u] = time;
        low[u] = time;
        time++;

        for (int v : adjlist[u]) {
            if (!visited[v]) {
                parent[v] = u;
                children++;
                APUtil(v, visited, ap, parent, low, disc);
                low[u] = std::min(low[u], low[v]);

                if (parent[u] == -1 && children > 1) {
                    ap[u] = true;
                }

                if (parent[u] != -1 && low[v] >= disc[u]) {
                    ap[u] = true;
                }
            } else if (v != parent[u]) {
                low[u] = std::min(low[u], disc[v]);
            }
        }
    }

    // Find articulation points / cut vertices using DFS
    void AP() {
        time = 0;
        std::vector<bool> visited(moduleCount, false);
        std::vector<int> disc(moduleCount, -1);
        std::vector<int> low(moduleCount, -1);
        std::vector<int> parent(moduleCount, -1);
        std::vector<bool> ap(moduleCount, false);

        for (int id = 0; id < moduleCount; id++) {
            if (!visited[id]) {
                APUtil(id, visited, ap, parent, low, disc);
            }
        }

        for (int id = 0; id < moduleCount; id++) {
            if (ap[id]) {
                auto& mod = ModuleIdManager::Modules()[id];
                DEBUG("Module at (" << mod.coords[0] << ", " << mod.coords[1] << ") is an articulation point" << std::endl);
                articulationPoints.emplace_back(mod.coords);
            }
        }
    }

    bool operator==(const Lattice& other) {
        bool result = false;
        if (grid.size() == other.grid.size()) {
            result = true;
            for (int i = 0; i < grid.size(); i++) {
                if (grid[i] != other.grid[i]) {
                    return false;
                }
            }
        }
        return result;
    }

    friend std::ostream& operator<<(std::ostream& out, /*const*/ Lattice& mod);
};

std::ostream& operator<<(std::ostream& out, /*const*/ Lattice& lattice) {
    out << "Lattice State:\n";
    for (int i = 0; i < lattice.coordTensor.size(); i++) {
        if (lattice.coordTensor.GetIdDirect(i) >= 0) {
            out << '#';
        } else {
            out << '-';
        }
        if ((i + 1) % lattice.axisSize == 0) {
            out << '\n';
        }
    }
    return out;
}

namespace Move {
    enum State {
        EMPTY = 'x',
        INITIAL = '?',
        FINAL = '!',
        STATIC = '#'
    };
}

class IMove {
public:
    // Load in move info from a given file
    virtual void InitMove(std::ifstream& moveFile) = 0;
    // Check to see if move is possible for a given module
    virtual bool MoveCheck(CoordTensor& tensor, const Module& mod) = 0;
};

class Move2d : IMove {
private:
    std::vector<std::pair<std::valarray<int>, bool>> moves;
    int yMultiplier;
    std::valarray<int> initPos, finalPos;

public:
    explicit Move2d(int axisSize) : yMultiplier(axisSize) {}

    void InitMove(std::ifstream& moveFile) override {
        int x = 0, y = 0;
        std::string line;
        while (std::getline(moveFile, line)) {
            for (auto c : line) {
                if (c == Move::EMPTY) {
                    moves.push_back({{x, y}, false});
                } else if (c == Move::STATIC) {
                    moves.push_back({{x, y}, true});
                } else if (c == Move::FINAL) {
                    moves.push_back({{x, y}, false});
                    finalPos = {x, y};
                } else if (c == Move::INITIAL) {
                    initPos = {x, y};
                }
                x++;
            }
            x = 0;
            y++;
        }
        for (auto& move : moves) {
            move.first -= initPos;
            DEBUG("Check Offset: " << move.first[0] << ", " << move.first[1] << (move.second ? " Static" : " Empty") << std::endl);
        }
        finalPos -= initPos;
        DEBUG("Move Offset: " << finalPos[0] << ", " << finalPos[1] << std::endl);
    }

    bool MoveCheck(CoordTensor& tensor, const Module& mod) override {
        for (const auto& move : moves) {
            if ((tensor[mod.coords + move.first] < 0) == move.second) {
                return false;
            }
        }
        return true;
    }

    const std::valarray<int>& MoveOffset() {
        return finalPos;
    }

    void rotateMove() {

    }
};

class Move3d : IMove {
private:
    std::vector<std::pair<std::valarray<int>, bool>> moves;
    std::valarray<int> initPos, finalPos;

public:
    void InitMove(std::ifstream& moveFile) override {
        int x = 0, y = 0, z = 0;
        std::string line;
        while (std::getline(moveFile, line)) {
            if (line.empty()) {
                z++;
                y = 0;
                continue;
            }
            for (auto c : line) {
                if (c == Move::EMPTY) {
                    moves.push_back({{x, y, z}, false});
                } else if (c == Move::STATIC) {
                    moves.push_back({{x, y, z}, true});
                } else if (c == Move::FINAL) {
                    moves.push_back({{x, y, z}, false});
                    finalPos = {x, y, z};
                } else if (c == Move::INITIAL) {
                    initPos = {x, y, z};
                }
                x++;
            }
            x = 0;
            y++;
        }
        for (auto& move : moves) {
            move.first -= initPos;
            DEBUG("Check Offset: " << move.first[0] << ", " << move.first[1] << ", " << move.first[2] << (move.second ? " Static" : " Empty") << std::endl);
        }
        finalPos -= initPos;
        DEBUG("Move Offset: " << finalPos[0] << ", " << finalPos[1] << ", " << finalPos[2] << std::endl);
    }

    bool MoveCheck(CoordTensor& tensor, const Module& mod) override {
        for (const auto& move : moves) {
            if ((tensor[mod.coords + move.first] < 0) == move.second) {
                return false;
            }
        }
        return true;
    }

    const std::valarray<int>& MoveOffset() {
        return finalPos;
    }

    void rotateMove() {

    }
};

class State {
private:
    size_t seed;
public:
    State() : seed(0) {}

    State(size_t seed) : seed(seed) {}

    size_t getSeed() const { return seed; }

    /*
    pass in information about lattice (hash bool of where modules are or bitfield c++)
    return hash value
    */
    static std::size_t hashLattice(const Lattice& lattice) {
        return boost::hash_range(lattice.grid.begin(), lattice.grid.end());
    }

    bool compareStates(const State& other) const { return seed == other.getSeed(); }
    /*
    check how to do this properly (operator overload function in lattice function)
    return true iff lattice have the same structure of modules
    */
    bool compareLattice(const Lattice& Lattice1, const Lattice& Lattice2) {}
};

/*
run bfs on configuration space
return path of bfs via states taken
*/
std::vector<State*> bfs(const State& initialState, const State& finalState) {
}

int main() {
    int order = 2;
    int axisSize = 4;
    Lattice lattice(order, axisSize);
    const int ORIGIN = 0;
    int x = ORIGIN;
    int y = ORIGIN;
    std::vector<std::vector<char>> image;

    std::ifstream file("test1.txt");
    if (!file) {
        std::cerr << "Unable to open file test1.txt";
        return 1;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<char> row;
        for (char c: line) {
            if (c == '1') {
                std::valarray<int> coords = {x, y};
                lattice.addModule(coords);
                row.push_back('1');
            } else if (c == '0') {
                row.push_back('0');
            }
            x++;
        }
        image.push_back(row);
        x = ORIGIN;
        y++;
    }
    file.close();
    lattice.AP();
    for (auto i: lattice.articulationPoints) {
        image[i[1] - ORIGIN][i[0] - ORIGIN] = '*';
    }
    for (const auto& imageRow: image) {
        for (auto c: imageRow) {
            std::cout << c;
        }
        std::cout << std::endl;
    }

    //
    //  MOVE TESTING BELOW
    //
    std::cout << "Lattice Initial State:\n" << lattice;
    std::ifstream moveFile("Moves/Slide_1.txt");
    if (!moveFile) {
        std::cerr << "Unable to open file Moves/Slide_1.txt";
        return 1;
    }
    Move2d move(axisSize);
    move.InitMove(moveFile);
    bool test = move.MoveCheck(lattice.coordTensor, ModuleIdManager::Modules()[1]);
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[1], move.MoveOffset());
        std::cout << "Lattice Final State:\n" << lattice;
    }
    std::cout << "(Note: Lattice output is rotated 90 degrees counterclockwise for some reason)\n";
    //
    //  END TESTING
    //
    return 0;
}
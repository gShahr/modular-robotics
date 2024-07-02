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
#include <queue>
#include <unordered_set>

class Module;

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
    CoordTensor<bool> stateTensor;
    // CoordTensor, should eventually replace coordmat
    CoordTensor<int> coordTensor;
    // Holds coordinate info for articulation points / cut vertices
    std::vector<std::valarray<int>> articulationPoints;

    Lattice(int order, int axisSize) : stateTensor(order, axisSize, false), coordTensor(order, axisSize, -1), order(order), axisSize(axisSize), time(0), moduleCount(0) {}

    // Add a new module
    void addModule(const std::valarray<int>& coords) {
        stateTensor[{coords}] = true;
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
            if (!bothWays || adjCoords[i] + 2 == axisSize) {
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
    [[maybe_unused]]
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
        if (stateTensor.GetArrayInternal().size() == other.stateTensor.GetArrayInternal().size()) {
            result = true;
            // this local vector is ideally a temporary solution
            auto& otherArrInternal = other.stateTensor.GetArrayInternal();
            for (int i = 0; i < stateTensor.GetArrayInternal().size(); i++) {
                if (stateTensor.GetIdDirect(i) != otherArrInternal[i]) {
                    return false;
                }
            }
        }
        return result;
    }

    Lattice& operator=(CoordTensor<bool> coordTensor) {
        return *this;
        // TODO
    }

    friend std::ostream& operator<<(std::ostream& out, /*const*/ Lattice& lattice);
};

std::ostream& operator<<(std::ostream& out, /*const*/ Lattice& lattice) {
    out << "Lattice State:\n";
    for (int i = 0; i < lattice.coordTensor.GetArrayInternal().size(); i++) {
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
        NOCHECK = ' ',
        EMPTY = 'x',
        INITIAL = '?',
        FINAL = '!',
        STATIC = '#'
    };
}

class MoveBase {
protected:
    // each pair represents a coordinate offset to check and whether a module should be there or not
    std::vector<std::pair<std::valarray<int>, bool>> moves;
    // bounds ex: {(2, 1), (0, 1)} would mean bounds extend from -2 to 1 on x-axis and 0 to 1 on y-axis
    std::vector<std::pair<int, int>> bounds;
    std::valarray<int> initPos, finalPos;
    int order = -1;
public:
    // Create a copy of a move
    [[nodiscard]]
    virtual MoveBase* CopyMove() const = 0;
    // Load in move info from a given file
    virtual void InitMove(std::ifstream& moveFile) = 0;
    // Check to see if move is possible for a given module
    virtual bool MoveCheck(CoordTensor<int>& tensor, const Module& mod) = 0;

    void RotateMove(int index) {
        std::swap(initPos[0], initPos[index]);
        std::swap(finalPos[0], finalPos[index]);
        std::swap(bounds[0], bounds[index]);
        for (auto& move : moves) {
            std::swap(move.first[0], move.first[index]);
        }
    }

    void ReflectMove(int index) {
        initPos[index] *= -1;
        finalPos[index] *= -1;
        std::swap(bounds[index].first, bounds[index].second);
        for (auto& move : moves) {
            move.first[index] *= -1;
        }
    }

    const std::valarray<int>& MoveOffset() {
        return finalPos;
    }

    virtual ~MoveBase() = default;

    // MoveManager will need to see moves and offsets
    friend class MoveManager;
};

class MoveManager {
private:
    // Vector containing every move
    static std::vector<MoveBase*> _moves;
    // Vector containing only generated moves
    static std::vector<MoveBase*> _movesToFree;
public:
    // Never instantiate MoveManager
    MoveManager() = delete;
    MoveManager(const MoveManager&) = delete;

    // To be used to generate multiple moves from a single move
    static void GenerateMovesFrom(MoveBase* origMove) {
        std::vector<MoveBase*> movesGen;
        // Add initial move to working vector
        movesGen.push_back(origMove);
        // Add rotations to working vector
        for (int i = 1; i < origMove->order; i++) {
            auto moveRotated = origMove->CopyMove();
            moveRotated->RotateMove(i);
            movesGen.push_back(moveRotated);
            _movesToFree.push_back(moveRotated);
        }
        // Reflections
        for (int i = 0; i < origMove->order; i++) {
            auto movesToReflect = movesGen;
            for (auto move : movesToReflect) {
                auto moveReflected = move->CopyMove();
                moveReflected->ReflectMove(i);
                movesGen.push_back(moveReflected);
                _movesToFree.push_back(moveReflected);
            }
        }
        // Add everything to _moves
        for (auto move : movesGen) {
            _moves.push_back(move);
        }
    }

    // To be used when no additional moves should be generated
    static void RegisterSingleMove(MoveBase* move) {
        _moves.push_back(move);
    }

    static const std::vector<MoveBase*>& Moves() {
        return _moves;
    }

    static std::vector<MoveBase*> CheckAllMoves(CoordTensor<int>& tensor, Module& mod) {
        std::vector<MoveBase*> LegalMoves = {};
        for (auto move : _moves) {
            if (move->MoveCheck(tensor, mod)) {
                LegalMoves.push_back(move);
            }
        }
        return LegalMoves;
    }

    static void CleanMoves() {
        for (auto move : _movesToFree) {
            delete move;
        }
    }
};

std::vector<MoveBase*> MoveManager::_moves;
std::vector<MoveBase*> MoveManager::_movesToFree;

class Move2d : public MoveBase {
public:
    Move2d() {
        order = 2;
        bounds.resize(order, {0, 0});
    }

    [[nodiscard]]
    MoveBase* CopyMove() const override {
        auto copy = new Move2d();
        *copy = *this;
        return copy;
    }

    void InitMove(std::ifstream& moveFile) override {
        int x = 0, y = 0;
        std::valarray<int> maxBounds = {0, 0};
        std::string line;
        while (std::getline(moveFile, line)) {
            for (auto c : line) {
                if (c == Move::NOCHECK) {
                    x++;
                    continue;
                }
                if (c == Move::EMPTY) {
                    moves.push_back({{x, y}, false});
                } else if (c == Move::STATIC) {
                    moves.push_back({{x, y}, true});
                } else if (c == Move::FINAL) {
                    moves.push_back({{x, y}, false});
                    finalPos = {x, y};
                } else if (c == Move::INITIAL) {
                    initPos = {x, y};
                    bounds = {{x, 0}, {y, 0}};
                }
                if (x > maxBounds[0]) {
                    maxBounds[0] = x;
                }
                if (y > maxBounds[1]) {
                    maxBounds[1] = y;
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
        maxBounds -= initPos;
        bounds[0].second = maxBounds[0];
        bounds[1].second = maxBounds[1];
    }

    bool MoveCheck(CoordTensor<int>& tensor, const Module& mod) override {
        // Bounds checking
        for (int i = 0; i < order; i++) {
            if (mod.coords[i] - bounds[i].first < 0 || mod.coords[i] + bounds[i].second >= tensor.AxisSize()) {
                return false;
            }
        }
        // Move Check
        for (const auto& move : moves) {
            if ((tensor[mod.coords + move.first] < 0) == move.second) {
                return false;
            }
        }
        return true;
    }
};

class Move3d : public MoveBase {
public:
    Move3d() {
        order = 3;
        bounds.resize(3, {0, 0});
    }

    [[nodiscard]]
    MoveBase* CopyMove() const override {
        auto copy = new Move3d();
        *copy = *this;
        return copy;
    }

    void InitMove(std::ifstream& moveFile) override {
        int x = 0, y = 0, z = 0;
        std::valarray<int> maxBounds = {0, 0, 0};
        std::string line;
        while (std::getline(moveFile, line)) {
            if (line.empty()) {
                z++;
                y = 0;
                continue;
            }
            for (auto c : line) {
                if (c == Move::NOCHECK) {
                    x++;
                    continue;
                }
                if (c == Move::EMPTY) {
                    moves.push_back({{x, y, z}, false});
                } else if (c == Move::STATIC) {
                    moves.push_back({{x, y, z}, true});
                } else if (c == Move::FINAL) {
                    moves.push_back({{x, y, z}, false});
                    finalPos = {x, y, z};
                } else if (c == Move::INITIAL) {
                    initPos = {x, y, z};
                    bounds = {{x, 0}, {y, 0}, {z, 0}};
                }
                if (x > maxBounds[0]) {
                    maxBounds[0] = x;
                }
                if (y > maxBounds[1]) {
                    maxBounds[1] = y;
                }
                if (z > maxBounds[2]) {
                    maxBounds[2] = z;
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
        maxBounds -= initPos;
        bounds[0].second = maxBounds[0];
        bounds[1].second = maxBounds[1];
        bounds[2].second = maxBounds[2];
    }

    bool MoveCheck(CoordTensor<int>& tensor, const Module& mod) override {
        // Bounds checking
        for (int i = 0; i < order; i++) {
            if (mod.coords[i] - bounds[i].first < 0 || mod.coords[i] + bounds[i].second >= tensor.AxisSize()) {
                return false;
            }
        }
        // Move Check
        for (const auto& move : moves) {
            if ((tensor[mod.coords + move.first] < 0) == move.second) {
                return false;
            }
        }
        return true;
    }
};

class hashedNode {
private:
    size_t seed;
public:
    hashedNode() : seed(0) {}

    hashedNode(size_t seed) : seed(seed) {}

    size_t getSeed() const { return seed; }

    /*
    pass in information about lattice (hash bool of where modules are or bitfield c++)
    return hash value
    */
    static std::size_t hashLattice(const Lattice& lattice) {
        return boost::hash_range(lattice.stateTensor.GetArrayInternal().begin(), lattice.stateTensor.GetArrayInternal().end());
    }

    static std::size_t hashCoordTensor(const CoordTensor<bool>& coordTensor) {
        return boost::hash_range(coordTensor.GetArrayInternal().begin(), coordTensor.GetArrayInternal().end());
    }

    bool compareStates(const hashedNode& other) const { return seed == other.getSeed(); }
    /*
    check how to do this properly (operator overload function in lattice function)
    return true iff lattice have the same structure of modules
    */
    bool compareLattice(const Lattice& Lattice1, const Lattice& Lattice2) {}
};

class Configuration {
private:
    Configuration* parent;
    std::vector<Configuration*> next;
    CoordTensor<bool> state;
public:
    Configuration(CoordTensor<bool> state) { this->state = state; }

    std::vector<CoordTensor<bool>> makeAllMoves() {
        std::vector<CoordTensor<bool>> result;
        Lattice lattice = state;
        for (auto module: ModuleIdManager::Modules()) {
            // make sure cut vertices are updated
            result.emplace_back(makeMoves(lattice, module));
        }
        return result;
    }

    /*
    Returns upto 8 possible lattice configurations from given lattice per movable modules
    TODO
        Use coordTensor instead of lattice
    */
    CoordTensor<bool> makeMoves(Lattice& lattice, Module& module) {
        std::vector<Lattice> result;
        /* 
        TODO
            Use movemanager instead of reading from file
        */
        std::ifstream moveFile("Moves/Slide_1.txt");
        if (!moveFile) {
            std::cerr << "Unable to open file Moves/Slide_1.txt";
            return result;
        }
        Move2d move;
        move.InitMove(moveFile);
        moveFile.close();
        MoveManager::GenerateMovesFrom(&move);
        auto legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, module);
        for (auto move : legalMoves) {
            result.emplace_back(applyMove(lattice, module, *move));
        }
        return result;
    }

    /*
    TODO
        Refactor to use coordiante tensor instead of lattice
        Make different move module for coordTensor or implement entire movement using coordTensor in applyMove
    */
    CoordTensor<bool> applyMove(Lattice lattice, Module& module, MoveBase& move) {
        lattice.moveModule(module, move.MoveOffset());
        return lattice.coordTensor;
    }

    void addEdge(Configuration* configuration) {
        next.push_back(configuration);
    }

    Configuration* getParent() {
        return parent;
    }

    std::vector<Configuration*> getNext() {
        return next;
    }

    Lattice getLattice() {
        return lattice;
    }

    void setState(State state) {
        this->state = state;
    }

    void setParent(Configuration* configuration) {
        parent = configuration;
    }
};

class ConfigurationSpace {
private:
public:
    /*
    run bfs on configuration space
    return path of bfs via states taken
    */
    std::vector<Configuration*> bfs(Configuration* start, Configuration* final) {
        std::queue<Configuration*> q;
        std::unordered_set<hashedNode> visited;
        q.push(start);
        visited.insert(start);
        while (!q.empty()) {
            Configuration* current = q.front();
            q.pop();
            if (current == final) {
                return findPath(start, final);
            }
            auto adjList = current->makeAllMoves();
            for (auto node: adjList) {
                // check if hashed node is visited
                if (visited.find(hashedNode(node)) == visited.end()) {
                    Configuration nextConfiguration = new Configuration(node);
                    nextConfiguration.setParent(current);
                    q.push(nextConfiguration);
                    visited.insert(node);
                }
            }
        }
    }
    
    /*
    backtrack to find path from start to final
    return path of configurations
    */
    std::vector<Configuration*> findPath(Configuration* start, Configuration* final) {
        std::vector<Configuration*> path;
        Configuration* current = final;
        while (current != start) {
            path.push_back(current);
            current = current->getParent();
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }

    /*
    TODO
        Change lattice to coordtensors of bools
    */
    void generateNeighbors(Configuration* configuration) {
        std::vector<std::vector<Lattice>> lattices = configuration->makeAllMoves();
        for (auto latticeList: lattices) {
            for (auto lattice: latticeList) {
                Configuration* newConfiguration = new Configuration();
                newConfiguration->parent = configuration;
                newConfiguration->lattice = lattice;
                newConfiguration.setState(State::hashLattice(lattice));
                configuration->addEdge(newConfiguration);
            }
        }
    }
};

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
    std::cout << lattice;
    std::ifstream moveFile("Moves/Slide_1.txt");
    if (!moveFile) {
        std::cerr << "Unable to open file Moves/Slide_1.txt";
        return 1;
    }
    Move2d move;
    move.InitMove(moveFile);
    bool test = move.MoveCheck(lattice.coordTensor, ModuleIdManager::Modules()[0]);
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[0], move.MoveOffset());
        std::cout << lattice;
    }
    MoveManager::GenerateMovesFrom(&move);
    // movegen testing
    // test = move.MoveCheck(lattice.coordTensor, ModuleIdManager::Modules()[2]);
    auto legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[2]);
    test = !legalMoves.empty();
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[2], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }
    // test2
    legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[1]);
    test = !legalMoves.empty();
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[1], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }
    // test3
    legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[0]);
    test = !legalMoves.empty();
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[0], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }
    // test4
    legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[0]);
    test = !legalMoves.empty();
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[0], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }
    // test5
    legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[1]);
    test = !legalMoves.empty();
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[1], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }
    // test6
    legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[2]);
    test = !legalMoves.empty();
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[2], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }
    // test7
    legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[0]);
    test = !legalMoves.empty();
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[0], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }
    MoveManager::CleanMoves();
    //
    //  END TESTING
    //
    return 0;
}
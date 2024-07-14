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
#include <boost/format.hpp>
#include <queue>
#include <unordered_set>
#include <nlohmann/json.hpp>

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
    [[nodiscard]]
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
    // Static module check
    bool moduleStatic = false;
    // Module ID
    int id;

    explicit Module(const std::valarray<int>& coords, bool isStatic = false) : coords(coords), moduleStatic(isStatic), id(ModuleIdManager::GetNextId()) { }
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

class MoveBase {
protected:
    // each pair represents a coordinate offset to check and whether a module should be there or not
    std::vector<std::pair<std::valarray<int>, bool>> moves;
    // bounds ex: {(2, 1), (0, 1)} would mean bounds extend from -2 to 1 on x-axis and 0 to 1 on y-axis
    std::vector<std::pair<int, int>> bounds;
    std::valarray<int> initPos, finalPos, anchorPos;
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
        std::swap(anchorPos[0], anchorPos[index]);
        std::swap(bounds[0], bounds[index]);
        for (auto& move : moves) {
            std::swap(move.first[0], move.first[index]);
        }
    }

    void ReflectMove(int index) {
        initPos[index] *= -1;
        finalPos[index] *= -1;
        anchorPos[index] *= -1;
        std::swap(bounds[index].first, bounds[index].second);
        for (auto& move : moves) {
            move.first[index] *= -1;
        }
    }

    const std::valarray<int>& MoveOffset() {
        return finalPos;
    }

    const std::valarray<int>& AnchorOffset() {
        return anchorPos;
    }

    virtual ~MoveBase() = default;

    // MoveManager will need to see moves and offsets
    friend class MoveManager;
};

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
    // state tensor
    CoordTensor<bool> stateTensor;
    // CoordTensor, should eventually replace coordmat
    CoordTensor<int> coordTensor;
    // Holds coordinate info for articulation points / cut vertices
    std::vector<std::valarray<int>> articulationPoints;
    std::vector<Module*> nonCutModules;

    Lattice(int order, int axisSize) : stateTensor(order, axisSize, false), coordTensor(order, axisSize, -1), order(order), axisSize(axisSize), time(0), moduleCount(0) {}

    // Add a new module
    void addModule(const std::valarray<int>& coords, bool isStatic = false) {
        stateTensor[{coords}] = true;
        // Create and register new module
        Module mod(coords, isStatic);
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
        stateTensor.IdAt(mod.coords) = false;
        mod.coords += offset;
        coordTensor.IdAt(mod.coords) = mod.id;
        stateTensor.IdAt(mod.coords) = true;
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
            auto& mod = ModuleIdManager::Modules()[id];
            if (ap[id]) {
                DEBUG("Module at (" << mod.coords[0] << ", " << mod.coords[1] << ") is an articulation point" << std::endl);
                articulationPoints.emplace_back(mod.coords);
            } else if (!mod.moduleStatic) {
                nonCutModules.emplace_back(&mod);
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

    Lattice& operator=(const CoordTensor<bool>& state) {
        auto& stateArray = state.GetArrayInternal();
        std::queue<int> modsToMove;
        std::queue<int> destinations;
        for (int i = 0; i < stateArray.size(); i++) {
            // Search for state differences
            if (stateTensor.GetIdDirect(i) == stateArray[i]) continue;
            if (stateArray[i]) {
                // New state has module at this index, current state doesn't have one
                if (modsToMove.empty()) {
                    // Remember this location for when a mismatched module is found
                    destinations.push(i);
                } else {
                    // Move a mismatched module to this location
                    coordTensor.GetIdDirect(i) = modsToMove.front();
                    // TEST: Update module position variable
                    ModuleIdManager::Modules()[modsToMove.front()].coords = coordTensor.CoordsFromIndex(i);
                    // Update adjacency list
                    ClearAdjacencies(coordTensor.GetIdDirect(i));
                    edgeCheck(ModuleIdManager::Modules()[coordTensor.GetIdDirect(i)]);
                    // Pop ID stack
                    modsToMove.pop();
                }
            } else {
                // Current state has module at this index, new state doesn't have one
                if (destinations.empty()) {
                    // Remember this mismatched module for when a location is found
                    modsToMove.push(coordTensor.GetIdDirect(i));
                } else {
                    // Move this mismatched module to a location
                    coordTensor.GetIdDirect(destinations.front()) = coordTensor.GetIdDirect(i);
                    // TEST: Update module position variable
                    ModuleIdManager::Modules()[coordTensor.GetIdDirect(i)].coords = coordTensor.CoordsFromIndex(destinations.front());
                    // Update adjacency list
                    ClearAdjacencies(coordTensor.GetIdDirect(i));
                    edgeCheck(ModuleIdManager::Modules()[coordTensor.GetIdDirect(i)]);
                    // Pop index stack
                    destinations.pop();
                }
                // Set former module location to -1
                coordTensor.GetIdDirect(i) = -1;
            }
            stateTensor.GetIdDirect(i) = stateArray[i];
        }
        return *this;
    }

    void setState(const CoordTensor<bool>& newState) {
        Lattice::stateTensor = newState;
    }

    std::vector<Module*> getMovableModules() {
        nonCutModules.clear();
        AP();
        return nonCutModules;
    }

    friend std::ostream& operator<<(std::ostream& out, /*const*/ Lattice& lattice);
};

std::ostream& operator<<(std::ostream& out, /*const*/ Lattice& lattice) {
    out << "Lattice State:\n";
    for (int i = 0; i < lattice.coordTensor.GetArrayInternal().size(); i++) {
        auto id = lattice.coordTensor.GetIdDirect(i);
        if (id >= 0) {
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
        STATIC = '#',
        ANCHOR = '@'
    };
}

class MoveManager {
private:
    // Vector containing every move
    static std::vector<MoveBase*> _moves;
    // Map from offset to move
    static CoordTensor<std::vector<MoveBase*>> _movesByOffset;
    // Vector containing only generated moves
    static std::vector<MoveBase*> _movesToFree;
    // Vector containing all move offsets
    static std::vector<std::valarray<int>> _offsets;
public:
    // Never instantiate MoveManager
    MoveManager() = delete;
    MoveManager(const MoveManager&) = delete;

    // Need to set up order of offset tensor
    static void InitMoveManager(int order, int maxDistance) {
        _movesByOffset = std::move(CoordTensor<std::vector<MoveBase*>>(order, 2 * maxDistance, {}, std::valarray<int>(maxDistance, order)));
    }

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
            if (_movesByOffset[move->finalPos].empty()) {
                _offsets.push_back(move->finalPos);
            }
            _movesByOffset[move->finalPos].push_back(move);
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
                DEBUG("passed!\n");
                LegalMoves.push_back(move);
            } else {
                DEBUG("failed!\n");
            }
        }
        return LegalMoves;
    }

    static std::pair<Module*, MoveBase*> FindMoveToState(Lattice& lattice, const CoordTensor<bool>& state) {
        Module* modToMove = nullptr;
        // Find module to move
        for (int i = 0; i < lattice.stateTensor.GetArrayInternal().size(); i++) {
            if (lattice.stateTensor.GetIdDirect(i) != state.GetIdDirect(i) && !state.GetIdDirect(i)) {
                modToMove = &ModuleIdManager::Modules()[lattice.coordTensor.GetIdDirect(i)];
                break;
            }
        }
        if (modToMove == nullptr) {
            return {nullptr, nullptr};
        }
        auto& modCoords = modToMove->coords;
        for (auto& offset : _offsets) {
            // Find offset to move to
            if (!state[modCoords + offset]) continue;
            // Find move to get there
            for (auto move : _movesByOffset[offset]) {
                if (move->MoveCheck(lattice.coordTensor, *modToMove)) {
                    return {modToMove, move};
                }
            }
        }
        return {modToMove, nullptr};
    }

    static void CleanMoves() {
        for (auto move : _movesToFree) {
            delete move;
        }
    }
};

std::vector<MoveBase*> MoveManager::_moves;
CoordTensor<std::vector<MoveBase*>> MoveManager::_movesByOffset(1, 1, {});
std::vector<MoveBase*> MoveManager::_movesToFree;
std::vector<std::valarray<int>> MoveManager::_offsets;

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
                switch (c) {
                    default:
                        DEBUG("Unrecognized Move: " << c << std::endl);
                    case Move::NOCHECK:
                        x++;
                        continue;
                    case Move::FINAL:
                        finalPos = {x, y};
                    case Move::EMPTY:
                        moves.push_back({{x, y}, false});
                        break;
                    case Move::ANCHOR:
                        anchorPos = {x, y};
                    case Move::STATIC:
                        moves.push_back({{x, y}, true});
                        break;
                    case Move::INITIAL:
                        initPos = {x, y};
                        bounds = {{x, 0}, {y, 0}};
                        break;
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
        anchorPos -= initPos;
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
                switch (c) {
                    default:
                        DEBUG("Unrecognized Move: " << c << std::endl);
                    case Move::NOCHECK:
                        x++;
                        continue;
                    case Move::FINAL:
                        finalPos = {x, y, z};
                    case Move::EMPTY:
                        moves.push_back({{x, y, z}, false});
                        break;
                    case Move::ANCHOR:
                        anchorPos = {x, y, z};
                    case Move::STATIC:
                        moves.push_back({{x, y, z}, true});
                        break;
                    case Move::INITIAL:
                        initPos = {x, y, z};
                        bounds = {{x, 0}, {y, 0}, {z, 0}};
                        break;
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
        anchorPos -= initPos;
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

class HashedState {
private:
    size_t seed;
public:
    HashedState() : seed(0) {}

    HashedState(size_t seed) : seed(seed) {}

    HashedState(CoordTensor<bool> coordTensor) {
        hashCoordTensor(coordTensor);
    }

    HashedState(const HashedState& other) : seed(other.getSeed()) {}

    size_t getSeed() const { return seed; }
    /*
    pass in information about lattice (hash bool of where modules are or bitfield c++)
    return hash value
    */
    void hashLattice(const Lattice& lattice) {
        seed = boost::hash_range(lattice.stateTensor.GetArrayInternal().begin(), lattice.stateTensor.GetArrayInternal().end());
    }

    void hashCoordTensor(const CoordTensor<bool>& coordTensor) {
        seed = boost::hash_range(coordTensor.GetArrayInternal().begin(), coordTensor.GetArrayInternal().end());
    }

    bool compareStates(const HashedState& other) const { return seed == other.getSeed(); }
    /*
    check how to do this properly (operator overload function in lattice function)
    return true iff lattice have the same structure of modules
    */
    bool compareLattice(const Lattice& Lattice1, const Lattice& Lattice2) {}

    bool operator==(const HashedState& other) const {
        return seed == other.getSeed();
    }
};

namespace std {
    template<>
    struct hash<HashedState> {
        std::size_t operator()(const HashedState& state) const {
            return std::hash<int>()(state.getSeed());
        }
    };
}

struct MoveWithCoordTensor {
    CoordTensor<bool> tensor;
    MoveBase* lastMove;
};

class Configuration {
private:
    Configuration* parent = nullptr;
    std::vector<Configuration*> next;
    CoordTensor<bool> _state;
    HashedState hash;
public:
    int depth = 0;
    explicit Configuration(CoordTensor<bool> state) : _state(std::move(state)) {}

    ~Configuration() {
        for (auto i = next.rbegin(); i != next.rend(); i++) {
            delete(*i);
        }
    }

    std::vector<CoordTensor<bool>> makeAllMoves(Lattice& lattice) {
        std::vector<CoordTensor<bool>> result;
        lattice.setState(_state);
        std::vector<Module*> movableModules = lattice.getMovableModules();
        for (auto module: movableModules) {
            auto legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, *module);
            for (auto move : legalMoves) {
                lattice.moveModule(*module, move->MoveOffset());
                result.push_back(lattice.stateTensor);
                lattice.moveModule(*module, -move->MoveOffset());
            }
        }
        return result;
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

    CoordTensor<bool> getState() {
        return _state;
    }

    HashedState getHash() {
        return hash;
    }

    void setStateAndHash(const CoordTensor<bool>& state) {
        _state = state;
        hash = HashedState(state);
    }

    void setParent(Configuration* configuration) {
        parent = configuration;
    }

    friend std::ostream& operator<<(std::ostream& os, const Configuration& config) {
        os << "Configuration: " << config.hash.getSeed() << std::endl;
        return os;
    }
};

std::ostream& operator<<(std::ostream& os, const std::vector<Configuration*>& configs) {
    for (const auto* config : configs) {
        if (config) {
            os << *config << std::endl;
        }
    }
    return os;
}

class ConfigurationSpace {
private:
public:
    /*
    run bfs on configuration space
    return path of bfs via states taken
    */
    static std::vector<Configuration*> bfs(Configuration* start, Configuration* final, Lattice& lattice) {
        int testI = -1;
        std::queue<Configuration*> q;
        std::unordered_set<HashedState> visited;
        q.push(start);
        visited.insert(start->getHash());
        while (!q.empty()) {
            Configuration* current = q.front();
            lattice = q.front()->getState();
            if (q.front()->depth != testI) {
                testI++;
                std::cout << "bfs depth: " << q.front()->depth << std::endl << lattice << std::endl;
            }
            q.pop();
            if (current->getState() == final->getState()) {
                return findPath(start, current);
            }
            auto adjList = current->makeAllMoves(lattice);
            for (const auto& node: adjList) {
                if (visited.find(HashedState(node)) == visited.end()) {
                    auto nextConfiguration = new Configuration(node);
                    nextConfiguration->setParent(current);
                    nextConfiguration->setStateAndHash(node);
                    q.push(nextConfiguration);
                    current->addEdge(nextConfiguration);
                    nextConfiguration->depth = current->depth + 1;
                    visited.insert(node);
                }
            }
        }
        return {};
    }
    
    /*
    backtrack to find path from start to final
    return path of configurations
    */
    static std::vector<Configuration*> findPath(Configuration* start, Configuration* final) {
        std::vector<Configuration*> path;
        Configuration* current = final;
        while (current->getState() != start->getState()) {
            path.push_back(current);
            current = current->getParent();
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }
};

// This might be better off as a namespace
namespace Scenario {
    static void exportStateTensorToJson(int id, const CoordTensor<bool>& stateTensor, const std::string& filename) {
        int indentSize = 4;
        nlohmann::json jsonOutput;
        for (int i = 0; i < stateTensor.GetArrayInternal().size(); i++) {
            jsonOutput["configurations"][id]["state"].push_back(stateTensor.GetIdDirect(i));
        }
        std::ofstream file(filename);
        if (file.is_open()) {
            file << jsonOutput.dump(indentSize);
            file.close();
        }
    }

    static void exportConfigurationSpaceToJson(const std::vector<Configuration*>& path, const std::string& filename) {
        for (int i = 0; i < path.size(); i++) {
            exportStateTensorToJson(i, path[i]->getState(), filename);
        }
    }

    static void exportToScen(Lattice& lattice, const std::vector<Configuration*>& path, const std::string& filename) {
        // File setup
        std::ofstream file(filename);
        // Group Definitions
        file << "0, 244, 244, 0, 95\n";
        file << "1, 255, 255, 255, 85\n\n";
        // Module Definitions
        auto idLen = std::to_string(ModuleIdManager::Modules().size()).size();
        boost::format padding("%%0%dd, %s");
        boost::format modDef((padding % idLen %  "%d, %d, %d, %d").str());
        lattice = path[0]->getState();
        for (int id = 0; id < ModuleIdManager::Modules().size(); id++) {
            auto& mod = ModuleIdManager::Modules()[id];
            modDef % id % (mod.moduleStatic ? 1 : 0) % mod.coords[0] % mod.coords[1] % (mod.coords.size() > 2 ? mod.coords[2] : 0);
            file << modDef.str() << std::endl;
        }
        // Move Definitions
        file << std::endl;
        for (int i = 1; i < path.size(); i++) {
            auto movePair = MoveManager::FindMoveToState(lattice, path[i]->getState());
            if (movePair.second == nullptr) {
                std::cout << "Failed to generate scenario file, no move to next state found.\n";
                file.close();
                return;
            }
            auto modToMove = movePair.first;
            auto move = movePair.second;
            modDef % modToMove->id % lattice.coordTensor[modToMove->coords + move->AnchorOffset()] % move->MoveOffset()[0] % move->MoveOffset()[1] % (move->MoveOffset().size() > 2 ? move->MoveOffset()[2] : 0);
            file << modDef.str() << std::endl;
            lattice.moveModule(*modToMove, move->MoveOffset());
        }
        // File cleanup
        file.close();
    }
};

int main() {
    int order = 2;
    int axisSize = 6;
    Lattice lattice(order, axisSize);
    MoveManager::InitMoveManager(order, axisSize);
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
                row.push_back('#');
            } else if (c == '0') {
                row.push_back(' ');
            } else if (c == '@') {
                std::valarray<int> coords = {x, y};
                lattice.addModule(coords, true);
                row.push_back('@');
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
    std::ifstream moveFile("Moves/Pivot_1.txt");
    if (!moveFile) {
        std::cerr << "Unable to open file Moves/Slide_1.txt";
        return 1;
    }
    Move2d move;
    move.InitMove(moveFile);
    MoveManager::GenerateMovesFrom(&move);
    std::ifstream moveFile2("Moves/Pivot_2.txt");
    if (!moveFile2) {
        std::cerr << "Unable to open file Moves/Slide_2.txt";
        return 1;
    }
    Move2d move2;
    move2.InitMove(moveFile2);
    MoveManager::GenerateMovesFrom(&move2);
    /*std::ifstream moveFile3("Moves/Leapfrog_1.txt");
    if (!moveFile3) {
        std::cerr << "Unable to open file Moves/Leapfrog_1.txt";
        return 1;
    }
    Move2d move3;
    move3.InitMove(moveFile3);
    MoveManager::GenerateMovesFrom(&move3);
    std::ifstream moveFile4("Moves/Leapfrog_2.txt");
    if (!moveFile4) {
        std::cerr << "Unable to open file Moves/Leapfrog_2.txt";
        return 1;
    }
    Move2d move4;
    move4.InitMove(moveFile4);
    MoveManager::GenerateMovesFrom(&move4);*/
    std::ifstream moveFile5("Moves/Monkey_1.txt");
    if (!moveFile5) {
        std::cerr << "Unable to open file Moves/Monkey_1.txt";
        return 1;
    }
    Move2d move5;
    move5.InitMove(moveFile5);
    MoveManager::GenerateMovesFrom(&move5);
    std::ifstream moveFile6("Moves/Monkey_2.txt");
    if (!moveFile6) {
        std::cerr << "Unable to open file Moves/Monkey_2.txt";
        return 1;
    }
    Move2d move6;
    move6.InitMove(moveFile6);
    MoveManager::GenerateMovesFrom(&move6);
    moveFile.close();
    moveFile2.close();
    //moveFile3.close();
    //moveFile4.close();
    moveFile5.close();
    moveFile6.close();
    /*
    auto legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[0]);
    bool test = !legalMoves.empty();
    std::cout << (test ? "MoveCheck Passed!" : "MoveCheck Failed!") << std::endl;
    moveFile.close();
    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[0], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }
    // movegen testing
    // test = move.MoveCheck(lattice.coordTensor, ModuleIdManager::Modules()[2]);
    legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, ModuleIdManager::Modules()[2]);
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

    if (test) {
        std::cout << "Moving!\n";
        lattice.moveModule(ModuleIdManager::Modules()[0], legalMoves[0]->MoveOffset());
        std::cout << lattice;
    }

    std::ifstream moveFile2("Moves/Pivot_1.txt");
    if (!moveFile2) {
        std::cerr << "Unable to open file Moves/Pivot_1.txt";
        return 1;
    }
    Move2d move2;
    move2.InitMove(moveFile2);
    MoveManager::GenerateMovesFrom(&move2);
    //
    //  END TESTING
    //

    // STATE TENSOR ASSIGNMENT TESTING
    std::cout << "Attempting to assign to lattice from state tensor.\n";
    CoordTensor<bool> stateTest(order, axisSize, false);
    stateTest[{1, 1}] = true;
    stateTest[{1, 2}] = true;
    stateTest[{2, 2}] = true;
    lattice = stateTest;
    std::cout << lattice;*/

    // BFS TESTING
    std::cout << "BFS Testing:\n";
    std::cout << "Original:    Desired:\n" <<
                 "  ----         --##\n" <<
                 "  -#--         ---#\n" <<
                 "  -##-         ----\n" <<
                 "  ----         ----\n";
    Configuration start(lattice.stateTensor);
    CoordTensor<bool> desiredState(order, axisSize, false);
    desiredState = lattice.stateTensor;
    /*
    desiredState[{3,3}] = false;
    desiredState[{4,3}] = false;
    desiredState[{4,4}] = false;
    desiredState[{4,5}] = false;
    desiredState[{5,5}] = false;
    desiredState[{8,3}] = true;
    desiredState[{8,4}] = true;
    desiredState[{7,4}] = true;
    desiredState[{6,4}] = true;
    desiredState[{6,5}] = true;
    */
    desiredState[{0,1}] = false;
    desiredState[{0,2}] = false;
    desiredState[{0,3}] = false;
    desiredState[{0,4}] = false;
    desiredState[{1,5}] = true;
    desiredState[{2,5}] = true;
    desiredState[{3,5}] = true;
    desiredState[{4,5}] = true;
    Configuration end(desiredState);
    auto path = ConfigurationSpace::bfs(&start, &end, lattice);
    std::cout << "Path:\n";
    for (auto config : path) {
        lattice = config->getState();
        std::cout << lattice;
    }
    Scenario::exportToScen(lattice, path, "test.scen");

    // Cleanup
    MoveManager::CleanMoves();
    return 0;
}
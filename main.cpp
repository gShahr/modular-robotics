#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <fstream>
#include <set>
#include <string>
#include "MoveManager.h"
#include "debug_util.h"
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <queue>
#include <unordered_set>
#include <nlohmann/json.hpp>

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
        lattice =_state;
        std::vector<Module*> movableModules = lattice.MovableModules();
        for (auto module: movableModules) {
            auto legalMoves = MoveManager::CheckAllMoves(lattice.coordTensor, *module);
            for (auto move : legalMoves) {
                lattice.MoveModule(*module, move->MoveOffset());
                result.push_back(lattice.stateTensor);
                lattice.MoveModule(*module, -move->MoveOffset());
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
            lattice.MoveModule(*modToMove, move->MoveOffset());
        }
        // File cleanup
        file.close();
    }
};

int main() {
    int order = 2;
    int axisSize = 9;
    Lattice lattice(order, axisSize);
    MoveManager::InitMoveManager(order, axisSize);
    const int ORIGIN = 0;
    int x = ORIGIN;
    int y = ORIGIN;
    std::vector<std::vector<char>> image;

    std::ifstream file("test2.txt");
    if (!file) {
        std::cerr << "Unable to open file test2.txt";
        return 1;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<char> row;
        for (char c: line) {
            if (c == '1') {
                std::valarray<int> coords = {x, y};
                lattice.AddModule(coords);
                row.push_back('#');
            } else if (c == '0') {
                row.push_back(' ');
            } else if (c == '@') {
                std::valarray<int> coords = {x, y};
                lattice.AddModule(coords, true);
                row.push_back('@');
            }
            x++;
        }
        image.push_back(row);
        x = ORIGIN;
        y++;
    }
    file.close();
    lattice.BuildMovableModules();
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
    /*std::ifstream moveFile5("Moves/Monkey_1.txt");
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
    MoveManager::GenerateMovesFrom(&move6);*/
    moveFile.close();
    moveFile2.close();
    //moveFile3.close();
    //moveFile4.close();
    //moveFile5.close();
    //moveFile6.close();
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

    /*desiredState[{0,1}] = false;
    desiredState[{0,2}] = false;
    desiredState[{0,3}] = false;
    desiredState[{0,4}] = false;
    desiredState[{1,5}] = true;
    desiredState[{2,5}] = true;
    desiredState[{3,5}] = true;
    desiredState[{4,5}] = true;*/
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
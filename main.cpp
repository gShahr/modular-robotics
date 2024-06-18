#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <fstream>
#include <set>
// CoordTensor not used yet, but it's all set up, just need to refactor main.cpp
#include "CoordTensor.h"
#include "debug_util.h"

class Module;

class ModuleIdManager {
private:
    static int _nextId;
    static std::vector<Module> _modules;

public:
    static void RegisterModule(Module& module) {
        _modules.emplace_back(module);
    }

    static int GetNextId() {
        return _nextId++;
    }

    static const std::vector<Module>& Modules() {
        return _modules;
    }
};

int ModuleIdManager::_nextId = 0;
std::vector<Module> ModuleIdManager::_modules;

class Module {
public:
    std::vector<int> coords;
    int id;

    Module(std::vector<int> coords) : coords(coords), id(ModuleIdManager::GetNextId()) { }
};

class Lattice {
private:
    std::map<std::vector<int>, int> coordmat;
    std::vector<std::vector<int>> adjlist;
    int time;
    int moduleCount;
    int width;
    int height;

public:
    CoordTensor coordTensor;
    std::vector<std::vector<int>> articulationPoints;

    Lattice(int order, int axisSize) : coordTensor(order, axisSize), time(0), moduleCount(0) {}

    void addModule(std::vector<int> coords) {
        Module mod(coords);
        ModuleIdManager::RegisterModule(mod);
        coordmat[{coords}] = ModuleIdManager::Modules()[moduleCount].id;
        edgeCheck(ModuleIdManager::Modules()[moduleCount], {});
        moduleCount++;
        adjlist.resize(moduleCount + 1);
    }

    void edgeCheck(const Module& mod, std::vector<int> coords) {
        if (coords.size() >= mod.coords.size()) {
            if (coordmat.count(coords)) {
                addEdge(mod.id, coordmat[{coords}]);
            }
        }
        int i = coords.size();
        coords.push_back(mod.coords[i]+1);
        edgeCheck(mod, coords);
        coords.pop_back();
        coords.push_back(mod.coords[i]-1);
        edgeCheck(mod, coords);
    }

    void edgeCheck2D(const Module& mod) {
        if (coordmat.count({mod.coords[0] - 1, mod.coords[1]}) != 0) {
            DEBUG("Module at " << mod.coords[0] << ", " << mod.coords[1] << " Adjacent to module at " << mod.coords[0] - 1 << ", " << mod.coords[1] << std::endl);
            addEdge(mod.id, coordmat[{mod.coords[0] - 1, mod.coords[1]}]);
        }
        if (coordmat.count({mod.coords[0], mod.coords[1] - 1}) != 0) {
            DEBUG("Module at " << mod.coords[0] << ", " << mod.coords[1] << " Adjacent to module at " << mod.coords[0] << ", " << mod.coords[1] - 1 << std::endl);
            addEdge(mod.id, coordmat[{mod.coords[0], mod.coords[1] - 1}]);
        }
    }

    void addEdge(int u, int v) {
        adjlist[u].push_back(v);
        adjlist[v].push_back(u);
    }

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
};

class Move {
private:
    enum State {
        EMPTY,
        INITIAL,
        FINAL,
        STATIC
    };
    std::set<std::map<std::vector<int>, State>> moves;
public:
    void readMove() {
        
    }

    void rotateMove() {

    }
};

int main() {
    int order = 2;
    int axisSize = 9;
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
                std::vector<int> coords = {x, y};
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
    return 0;
}
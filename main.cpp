#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <fstream>
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
    int id;
    int x, y;

    Module(int x, int y) : x(x), y(y), id(ModuleIdManager::GetNextId()) { }

    std::pair<int, int> getPosition() {
        return {x, y};
    }
};

class Lattice {
private:
    std::map<std::pair<int, int>, int> coordmat;
    std::vector<std::vector<int>> adjlist;
    int time;
    int moduleCount;
    int width;
    int height;

public:
    std::vector<std::pair<int, int>> articulationPoints;

    enum State {
        EMPTY,
        INITIAL,
        FINAL,
        STATIC
    };

    Lattice() : time(0), moduleCount(0) {}

    void addModule(int x, int y) {
        Module mod(x, y);
        ModuleIdManager::RegisterModule(mod);
        coordmat[{x, y}] = ModuleIdManager::Modules()[moduleCount].id;
        edgeCheck(ModuleIdManager::Modules()[moduleCount]);
        moduleCount++;
        adjlist.resize(moduleCount + 1);
    }

    void edgeCheck(const Module& mod) {
        if (coordmat.count({mod.x - 1, mod.y}) != 0) {
            DEBUG("Module at " << mod.x << ", " << mod.y << " Adjacent to module at " << mod.x - 1 << ", " << mod.y << std::endl);
            addEdge(mod.id, coordmat[{mod.x - 1, mod.y}]);
        }
        if (coordmat.count({mod.x, mod.y - 1}) != 0) {
            DEBUG("Module at " << mod.x << ", " << mod.y << " Adjacent to module at " << mod.x << ", " << mod.y - 1 << std::endl);
            addEdge(mod.id, coordmat[{mod.x, mod.y - 1}]);
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
                std::cout << "Module at (" << mod.x << ", " << mod.y << ") is an articulation point\n";
                articulationPoints.emplace_back(mod.x, mod.y);
            }
        }
    }
};

int main() {
    Lattice lattice;
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
                lattice.addModule(x, y);
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
        image[i.second - ORIGIN][i.first - ORIGIN] = '*';
    }
    for (const auto& imageRow: image) {
        for (auto c: imageRow) {
            std::cout << c;
        }
        std::cout << std::endl;
    }
    return 0;
}
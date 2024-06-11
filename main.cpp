#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <fstream>
#include "debug_util.h"

class Module {
public:
    int x, y;

    Module(int x, int y) : x(x), y(y) {}

    std::pair<int, int> getPosition() {
        return std::make_pair(x, y);
    }
};

class Lattice {
private:
    std::map<std::pair<int, int>, Module*> coordmat;
    std::map<Module*, std::vector<Module*>> adjlist;
    int time;
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

    Lattice() : time(0) {}

    void addModule(int x, int y) {
        Module* mod = new Module(x, y);
        coordmat[std::make_pair(x, y)] = mod;
        edgeCheck(mod);
    }

    void edgeCheck(Module* mod) {
        if (coordmat.count(std::make_pair(mod->x - 1, mod->y)) != 0) {
            DEBUG("Module at " << mod->x << ", " << mod->y << " Adjacent to module at " << mod->x - 1 << ", " << mod->y << std::endl);
            addEdge(mod, coordmat[std::make_pair(mod->x - 1, mod->y)]);
        }
        if (coordmat.count(std::make_pair(mod->x, mod->y - 1)) != 0) {
            DEBUG("Module at " << mod->x << ", " << mod->y << " Adjacent to module at " << mod->x << ", " << mod->y - 1 << std::endl);
            addEdge(mod, coordmat[std::make_pair(mod->x, mod->y - 1)]);
        }
    }

    void addEdge(Module* u, Module* v) {
        adjlist[u].push_back(v);
        adjlist[v].push_back(u);
    }

    void APUtil(Module* u, std::map<Module*, bool>& visited, std::map<Module*, bool>& ap, std::map<Module*, Module*>& parent, std::map<Module*, int>& low, std::map<Module*, int>& disc) {
        int children = 0;
        visited[u] = true;
        disc[u] = time;
        low[u] = time;
        time++;

        for (Module* v : adjlist[u]) {
            if (!visited[v]) {
                parent[v] = u;
                children++;
                APUtil(v, visited, ap, parent, low, disc);
                low[u] = std::min(low[u], low[v]);

                if (parent[u] == nullptr && children > 1) {
                    ap[u] = true;
                }

                if (parent[u] != nullptr && low[v] >= disc[u]) {
                    ap[u] = true;
                }
            } else if (v != parent[u]) {
                low[u] = std::min(low[u], disc[v]);
            }
        }
    }

    void AP() {
        time = 0;
        std::map<Module*, bool> visited;
        std::map<Module*, int> disc;
        std::map<Module*, int> low;
        std::map<Module*, Module*> parent;
        std::map<Module*, bool> ap;

        for (auto& kv : adjlist) {
            if (!visited[kv.first]) {
                APUtil(kv.first, visited, ap, parent, low, disc);
            }
        }

        for (auto& kv : ap) {
            if (kv.second) {
                std::cout << "Module at (" << kv.first->getPosition().first << ", " << kv.first->getPosition().second << ") is an articulation point\n";
                articulationPoints.emplace_back(kv.first->getPosition().first, kv.first->getPosition().second);
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
            } else {
                row.push_back('0');
            }
            x++;
        }
        image.push_back(row);
        x = 0;
        y++;
    }
    file.close();
    lattice.AP();
    for (auto i: lattice.articulationPoints) {
        image[i.second][i.first] = '*';
    }
    for (auto imageRow: image) {
        for (auto c: imageRow) {
            std::cout << c;
        }
        std::cout << std::endl;
    }
    return 0;
}
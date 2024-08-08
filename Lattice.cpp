#include <queue>
#include <sstream>
#include <string>
#include <map>
#include "debug_util.h"
#include "Colors.h"
#include "Lattice.h"

std::vector<std::vector<int>> Lattice::adjList;
int Lattice::order;
int Lattice::axisSize;
int Lattice::time = 0;
int Lattice::moduleCount = 0;
bool Lattice::ignoreProperties = false;
std::vector<Module*> Lattice::movableModules;
CoordTensor<bool> Lattice::stateTensor(1, 1, false);
CoordTensor<int> Lattice::coordTensor(1, 1, -1);

void Lattice::ClearAdjacencies(int moduleId) {
    for (int id : adjList[moduleId]) {
        for (int i = 0; i < adjList[id].size(); i++) {
            if (adjList[id][i] == moduleId) {
                adjList[id].erase(adjList[id].begin() + i);
                break;
            }
        }
    }
    adjList[moduleId].clear();
}

void Lattice::InitLattice(int _order, int _axisSize) {
    order = _order;
    axisSize = _axisSize;
    stateTensor = CoordTensor<bool>(order, axisSize, false);
    coordTensor = CoordTensor<int>(order, axisSize, -1);
}

void Lattice::setFlags(bool _ignoreColors) {
    ignoreProperties = _ignoreColors;
}

void Lattice::AddModule(const Module& mod) {
    // Update state and coord tensor
    stateTensor[mod.coords] = true;
    coordTensor[mod.coords] = mod.id;
    // Adjacency check
    EdgeCheck(mod, true);
    moduleCount++;
    adjList.resize(moduleCount + 1);
}

void Lattice::MoveModule(Module &mod, const std::valarray<int>& offset) {
    ClearAdjacencies(mod.id);
    coordTensor[mod.coords] = -1;
    stateTensor[mod.coords] = false;
    mod.coords += offset;
    coordTensor[mod.coords] = mod.id;
    stateTensor[mod.coords] = true;
    EdgeCheck(mod);
    if (!ignoreProperties) {
        mod.properties.UpdateProperties(offset);
    }
}

void Lattice::EdgeCheck(const Module &mod, bool bothWays) {
    // Copy module coordinates to adjCoords
    auto adjCoords = mod.coords;
    for (int i = 0; i < order; i++) {
        // Don't want to check index -1
        if (adjCoords[i] == 0) continue;
        adjCoords[i]--;
        if (coordTensor[adjCoords] >= 0) {
#if (LATTICE_VERBOSE & LAT_LOG_ADJ) == LAT_LOG_ADJ
            DEBUG(mod << " Adjacent to " << ModuleIdManager::Modules()[coordTensor[adjCoords]] << std::endl);
#endif
            AddEdge(mod.id, coordTensor[adjCoords]);
        }
        // Don't want to check both ways if it can be avoided, also don't want to check index beyond max value
        if (!bothWays || adjCoords[i] + 2 == axisSize) {
            adjCoords[i]++;
            continue;
        }
        adjCoords[i] += 2;
        if (coordTensor[adjCoords] >= 0) {
#if (LATTICE_VERBOSE & LAT_LOG_ADJ) == LAT_LOG_ADJ
            DEBUG(mod << " Adjacent to " << ModuleIdManager::Modules()[coordTensor[adjCoords]] << std::endl);
#endif
            AddEdge(mod.id, coordTensor[adjCoords]);
        }
        adjCoords[i]--;
    }
}

void Lattice::AddEdge(int modA, int modB) {
    adjList[modA].push_back(modB);
    adjList[modB].push_back(modA);
}

void Lattice::APUtil(int u, std::vector<bool> &visited, std::vector<bool> &ap, std::vector<int> &parent,
                     std::vector<int> &low, std::vector<int> &disc) {
    int children = 0;
    visited[u] = true;
    disc[u] = time;
    low[u] = time;
    time++;

    for (int v : adjList[u]) {
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

void Lattice::BuildMovableModules() {
    time = 0;
    std::vector<bool> visited(moduleCount, false);
    std::vector<int> disc(moduleCount, -1);
    std::vector<int> low(moduleCount, -1);
    std::vector<int> parent(moduleCount, -1);
    std::vector<bool> ap(moduleCount, false);
    movableModules.clear();

    for (int id = 0; id < moduleCount; id++) {
        if (!visited[id]) {
            APUtil(id, visited, ap, parent, low, disc);
        }
    }

    for (int id = 0; id < moduleCount; id++) {
        auto& mod = ModuleIdManager::Modules()[id];
        if (ap[id]) {
#if (LATTICE_VERBOSE & LAT_LOG_CUT) == LAT_LOG_CUT
            DEBUG(mod << " is an articulation point" << std::endl);
#endif
        } else if (!mod.moduleStatic) {
            // Non-cut, non-static modules
            movableModules.emplace_back(&mod);
        }
    }
}

void Lattice::BuildMovableModulesNonRec() {
    // Clear movableModules vector
    movableModules.clear();

    // Find articulation points non-recursively
    int t = 0;
    std::vector<bool> ap(moduleCount, false);
    std::vector<bool> visited(moduleCount, false);
    // this outer for loop I'm pretty sure isn't needed since the modules should all be connected
    for (int id = 0; id < moduleCount; id++) {
        if (visited[id]) {
            continue;
        }
        // can probably convert these to vectors
        std::vector<int> discovery(moduleCount, 0);
        std::vector<int> low(moduleCount, 0);
        int root_children = 0;
        visited[id] = true;
        std::stack<std::tuple<int, int, std::vector<int>::const_iterator>> stack;
        stack.emplace(id, id, adjList[id].cbegin());

        while (!stack.empty()) {
            auto [grandparent, parent, children] = stack.top();
            if (children != adjList[parent].cend()) {
                int child = *children;
                ++std::get<std::vector<int>::const_iterator>(stack.top());
                //++children;

                if (grandparent == child) {
                    continue;
                }

                if (visited[child]) {
                    if (discovery[child] <= discovery[parent]) {
                        low[parent] = discovery[child] < low[parent] ? discovery[child] : low[parent];
                    }
                } else {
                    t++;
                    low[child] = discovery[child] = t;
                    visited[child] = true;
                    stack.emplace(parent, child, adjList[child].cbegin());
                }
            } else {
                stack.pop();
                if (stack.size() > 1) {
                    if (low[parent] >= discovery[grandparent]) {
                        ap[grandparent] = true;
                    }
                    low[grandparent] = low[parent] < low[grandparent] ? low[parent] : low[grandparent];
                } else if (!stack.empty()) {
                    root_children++;
                }
            }
        }
        if (root_children > 1) {
            ap[id] = true;
        }
    }

    // Rebuild movableModules vector
    for (int id = 0; id < ModuleIdManager::MinStaticID(); id++) {
        if (!ap[id]) {
            movableModules.push_back(&ModuleIdManager::GetModule(id));
        }
    }
}

#define AP_Recursive false
const std::vector<Module*>& Lattice::MovableModules() {
#if AP_Recursive
    BuildMovableModules();
#else
    BuildMovableModulesNonRec();
#endif
    return movableModules;
}

void Lattice::UpdateFromModuleInfo(const std::set<ModuleBasic>& moduleInfo) {
    std::queue<const ModuleBasic*> destinations;
    std::unordered_set<int> modsToMove;
    for (int id = 0; id < ModuleIdManager::MinStaticID(); id++) {
        modsToMove.insert(id);
    }
    for (const auto& info : moduleInfo) {
        auto id = coordTensor[info.coords];
        auto& mod = ModuleIdManager::GetModule(id);
        if (id >= 0) {
            modsToMove.erase(id);
            if (mod.properties != info.properties) {
                mod.properties = info.properties;
            }
        } else {
            destinations.push(&info);
        }
    }
    if (modsToMove.size() != destinations.size()) {
        std::cerr << "Update partially completed due to state error, program likely non-functional!" << std::endl;
        return;
    }
    for (auto id : modsToMove) {
        auto& mod = ModuleIdManager::GetModule(id);
        Lattice::coordTensor[mod.coords] = -1;
        mod.coords = destinations.front()->coords;
        ClearAdjacencies(id);
        EdgeCheck(mod, true);
        Lattice::coordTensor[mod.coords] = mod.id;
        mod.properties = destinations.front()->properties;
        destinations.pop();
    }
}

std::set<ModuleBasic> Lattice::GetModuleInfo() {
    std::set<ModuleBasic> modInfo;
    for (int id = 0; id < ModuleIdManager::MinStaticID(); id++) {
        auto& mod = ModuleIdManager::GetModule(id);
        ModuleBasic modBasic = {mod.coords, mod.properties};
        modInfo.insert(modBasic);
    }
    return modInfo;
}

int Lattice::Order() {
    return order;
}

int Lattice::AxisSize() {
    return axisSize;
}

std::string Lattice::ToString() {
    std::stringstream out;
    if (order != 2) {
        DEBUG("Lattice string conversion not permitted for non-2d lattice");
        return "";
    }
    out << "Lattice State:\n";
    for (int i = 0; i < coordTensor.GetArrayInternal().size(); i++) {
        auto id = coordTensor.GetElementDirect(i);
        if (id >= 0 && !ignoreProperties) {
            auto colorProp = dynamic_cast<ColorProperty*>(ModuleIdManager::Modules()[id].properties.Find(COLOR_PROP_NAME));
            out << Colors::intToColor[colorProp->GetColorInt()][0];
        } else if (id >= 0) {
            out << '#';
        } else {
            out << '-';
        }
        if ((i + 1) % axisSize == 0) {
            out << '\n';
        }
    }
    return out.str();
}
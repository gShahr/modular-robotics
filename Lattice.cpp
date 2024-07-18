#include <queue>
#include "debug_util.h"
#include "Lattice.h"

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

Lattice::Lattice(int order, int axisSize) : stateTensor(order, axisSize, false),
                                            coordTensor(order, axisSize, -1), order(order), axisSize(axisSize) {}

void Lattice::AddModule(const std::valarray<int> &coords, bool isStatic) {
    // Create new module
    Module mod(coords, isStatic);
    // Update state and coord tensor
    stateTensor[coords] = true;
    coordTensor[coords] = mod.id;
    // Adjacency check
    EdgeCheck(mod, false);
    moduleCount++;
    adjList.resize(moduleCount + 1);
}

void Lattice::MoveModule(Module &mod, const std::valarray<int> &offset) {
    ClearAdjacencies(mod.id);
    coordTensor[mod.coords] = -1;
    stateTensor[mod.coords] = false;
    mod.coords += offset;
    coordTensor[mod.coords] = mod.id;
    stateTensor[mod.coords] = true;
    EdgeCheck(mod);
}

void Lattice::EdgeCheck(const Module &mod, bool bothWays) {
    // Copy module coordinates to adjCoords
    auto adjCoords = mod.coords;
    for (int i = 0; i < order; i++) {
        // Don't want to check index -1
        if (adjCoords[i] == 0) continue;
        adjCoords[i]--;
        if (coordTensor[adjCoords] >= 0) {
            DEBUG(mod << " Adjacent to " << ModuleIdManager::Modules()[coordTensor[adjCoords]] << std::endl);
            AddEdge(mod.id, coordTensor[adjCoords]);
        }
        // Don't want to check both ways if it can be avoided, also don't want to check index beyond max value
        if (!bothWays || adjCoords[i] + 2 == axisSize) {
            adjCoords[i]++;
            continue;
        }
        adjCoords[i] += 2;
        if (coordTensor[adjCoords] >= 0) {
            DEBUG(mod << " Adjacent to " << ModuleIdManager::Modules()[coordTensor[adjCoords]] << std::endl);
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
            DEBUG("Module at (" << mod.coords[0] << ", " << mod.coords[1] << ") is an articulation point" << std::endl);
        } else if (!mod.moduleStatic) {
            // Non-cut, non-static modules
            movableModules.emplace_back(&mod);
        }
    }
}

const std::vector<Module*>& Lattice::MovableModules() {
    BuildMovableModules();
    return movableModules;
}

bool Lattice::operator==(const Lattice &other) {
    bool result = false;
    if (stateTensor.GetArrayInternal().size() == other.stateTensor.GetArrayInternal().size()) {
        result = true;
        for (int i = 0; i < stateTensor.GetArrayInternal().size(); i++) {
            if (stateTensor.GetElementDirect(i) != other.stateTensor.GetElementDirect(i)) {
                return false;
            }
        }
    }
    return result;
}

Lattice& Lattice::operator=(const CoordTensor<bool> &state) {
    std::queue<int> modsToMove;
    std::queue<int> destinations;
    for (int i = 0; i < state.GetArrayInternal().size(); i++) {
        // Search for state differences
        if (stateTensor.GetElementDirect(i) == state.GetElementDirect(i)) continue;
        if (state.GetElementDirect(i)) {
            // New state has module at this index, current state doesn't have one
            if (modsToMove.empty()) {
                // Remember this location for when a mismatched module is found
                destinations.push(i);
            } else {
                // Move a mismatched module to this location
                coordTensor.GetElementDirect(i) = modsToMove.front();
                // TEST: Update module position variable
                ModuleIdManager::Modules()[modsToMove.front()].coords = coordTensor.CoordsFromIndex(i);
                // Update adjacency list
                ClearAdjacencies(coordTensor.GetElementDirect(i));
                EdgeCheck(ModuleIdManager::Modules()[coordTensor.GetElementDirect(i)]);
                // Pop ID stack
                modsToMove.pop();
            }
        } else {
            // Current state has module at this index, new state doesn't have one
            if (destinations.empty()) {
                // Remember this mismatched module for when a location is found
                modsToMove.push(coordTensor.GetElementDirect(i));
            } else {
                // Move this mismatched module to a location
                coordTensor.GetElementDirect(destinations.front()) = coordTensor.GetElementDirect(i);
                // TEST: Update module position variable
                ModuleIdManager::Modules()[coordTensor.GetElementDirect(i)].coords = coordTensor.CoordsFromIndex(destinations.front());
                // Update adjacency list
                ClearAdjacencies(coordTensor.GetElementDirect(i));
                EdgeCheck(ModuleIdManager::Modules()[coordTensor.GetElementDirect(i)]);
                // Pop index stack
                destinations.pop();
            }
            // Set former module location to -1
            coordTensor.GetElementDirect(i) = -1;
        }
        stateTensor.GetElementDirect(i) = state.GetElementDirect(i);
    }
    return *this;
}

std::ostream& operator<<(std::ostream& out, Lattice& lattice) {
    if (lattice.order != 2) {
        DEBUG("Lattice insertion operator not permitted for non-2d lattices");
        return out;
    }
    out << "Lattice State:\n";
    for (int i = 0; i < lattice.coordTensor.GetArrayInternal().size(); i++) {
        auto id = lattice.coordTensor.GetElementDirect(i);
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
#include "HeuristicCache.h"
#include <queue>
#include <execution>
#include "../lattice/Lattice.h"
#include "../moves/MoveManager.h"

constexpr float INVALID_WEIGHT = 999;

IHeuristicCache::IHeuristicCache(): weightCache(Lattice::Order(), Lattice::AxisSize(), INVALID_WEIGHT) {}

float IHeuristicCache::operator[](const std::valarray<int> &coords) const {
    return weightCache[coords];
}

void ChebyshevHeuristicCache::ChebyshevEnqueueAdjacent(std::queue<SearchCoord>& coordQueue, const SearchCoord& coordInfo) {
    std::vector<std::valarray<int>> adjCoords;
    adjCoords.push_back(coordInfo.coords);
    for (int i = 0; i < Lattice::Order(); i++) {
        auto adjCoordsTemp = adjCoords;
        for (auto adj : adjCoordsTemp) {
            adj[i]--;
            adjCoords.push_back(adj);
            adj[i] += 2;
            adjCoords.push_back(adj);
        }
    }
    for (const auto& coord : adjCoords) {
        if (Lattice::coordTensor[coord] == OUT_OF_BOUNDS || Lattice::coordTensor[coord] >=
            ModuleIdManager::MinStaticID()) continue;
        coordQueue.push({coord, coordInfo.depth + 1});
    }
}

ChebyshevHeuristicCache::ChebyshevHeuristicCache(const std::set<ModuleData>& desiredState) {
    for (const auto& desiredModuleData : desiredState) {
        std::queue<SearchCoord> coordQueue;
        coordQueue.push({desiredModuleData.Coords(), 0});
        while (!coordQueue.empty()) {
            std::valarray<int> coords = coordQueue.front().coords;
            const auto depth = coordQueue.front().depth;
            if (const auto weight = weightCache[coords]; depth < weight) {
                weightCache[coords] = depth;
            } else if (depth > weight) {
                coordQueue.pop();
                continue;
            }
            ChebyshevEnqueueAdjacent(coordQueue, coordQueue.front());
            coordQueue.pop();
        }
    }
    std::cout << "Weight Cache:";
    for (int i = 0; i < weightCache.GetArrayInternal().size(); i++) {
        if (i % Lattice::AxisSize() == 0) std::cout << std::endl;
        if (weightCache.GetArrayInternal()[i] < 10) {
            std::cout << weightCache.GetArrayInternal()[i];
        } else if (weightCache.GetArrayInternal()[i] == INVALID_WEIGHT) {
            std::cout << "#";
        } else {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

void MoveOffsetHeuristicCache::MoveOffsetEnqueueAdjacent(std::queue<SearchCoord>& coordQueue, const SearchCoord& coordInfo) {
    std::vector<std::valarray<int>> adjCoords;
    adjCoords.push_back(coordInfo.coords);
    auto adjCoordsTemp = adjCoords;
    for (auto adj : adjCoordsTemp) {
        for (const auto& offset : MoveManager::_offsets) {
            if (std::any_of(std::execution::par_unseq ,MoveManager::_movesByOffset[offset].begin(), MoveManager::_movesByOffset[offset].end(), [&](MoveBase* move) {
                return move->FreeSpaceCheck(Lattice::coordTensor, adj);
            })) {
                adj += offset;
                adjCoords.push_back(adj);
                adj -= offset;
            }
        }
    }
    for (const auto& coord : adjCoords) {
        if (Lattice::coordTensor[coord] == OUT_OF_BOUNDS || Lattice::coordTensor[coord] >=
            ModuleIdManager::MinStaticID()) continue;
        coordQueue.push({coord, coordInfo.depth + 1});
    }
}

MoveOffsetHeuristicCache::MoveOffsetHeuristicCache(const std::set<ModuleData> &desiredState) {
    // Temporarily remove non-static modules from lattice
    for (const auto& mod : ModuleIdManager::FreeModules()) {
        Lattice::coordTensor[mod.coords] = FREE_SPACE;
    }
    // Populate weight tensor
    for (const auto& desiredModuleData : desiredState) {
        std::queue<SearchCoord> coordQueue;
        coordQueue.push({desiredModuleData.Coords(), 0});
        while (!coordQueue.empty()) {
            std::valarray<int> coords = coordQueue.front().coords;
            const auto depth = coordQueue.front().depth;
            if (const auto weight = weightCache[coords]; depth < weight) {
                weightCache[coords] = depth;
            } else if (depth > weight) {
                coordQueue.pop();
                continue;
            }
            MoveOffsetEnqueueAdjacent(coordQueue, coordQueue.front());
            coordQueue.pop();
        }
    }
    // Restore non-static module to lattice
    for (const auto& mod : ModuleIdManager::FreeModules()) {
        Lattice::coordTensor[mod.coords] = mod.id;
    }
    // Print weight tensor
    std::cout << "Weight Cache:";
    for (int i = 0; i < weightCache.GetArrayInternal().size(); i++) {
        if (i % Lattice::AxisSize() == 0) std::cout << std::endl;
        if (weightCache.GetArrayInternal()[i] < 10) {
            std::cout << weightCache.GetArrayInternal()[i];
        } else if (weightCache.GetArrayInternal()[i] == INVALID_WEIGHT && Lattice::coordTensor.GetElementDirect(i) >= ModuleIdManager::MinStaticID()) {
            std::cout << "#";
        } else {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

std::unordered_map<std::uint_fast64_t, int> MoveOffsetPropertyHeuristicCache::propConversionMap;

void MoveOffsetPropertyHeuristicCache::MoveOffsetPropertyEnqueueAdjacent(std::queue<SearchCoordProp>& coordPropQueue, const SearchCoordProp& coordPropInfo) {
    std::vector<std::valarray<int>> adjCoords;
    adjCoords.push_back(coordPropInfo.coords);
    auto adjCoordsTemp = adjCoords;
    for (auto adj : adjCoordsTemp) {
        for (const auto& offset : MoveManager::_offsets) {
            if (std::any_of(std::execution::par_unseq ,MoveManager::_movesByOffset[offset].begin(), MoveManager::_movesByOffset[offset].end(), [&](MoveBase* move) {
                return move->FreeSpaceCheck(Lattice::coordTensor, adj);
            })) {
                adj += offset;
                adjCoords.push_back(adj);
                adj -= offset;
            }
        }
    }
    for (const auto& coord : adjCoords) {
        if (Lattice::coordTensor[coord] == OUT_OF_BOUNDS || Lattice::coordTensor[coord] >=
            ModuleIdManager::MinStaticID()) continue;
        coordPropQueue.push({coord, coordPropInfo.depth + 1, coordPropInfo.propInt});
    }
}

MoveOffsetPropertyHeuristicCache::MoveOffsetPropertyHeuristicCache(const std::set<ModuleData> &desiredState) {
    // Determine # of unique properties and map potentially large int representations to small values
    int propIndex = 0;
    for (const auto& desiredModuleData : desiredState) {
        if (!propConversionMap.contains(desiredModuleData.Properties().AsInt())) {
            propConversionMap[desiredModuleData.Properties().AsInt()] = propIndex;
            propIndex++;
        }
    }
    // Resize weight cache to account for property axis (increase order by 1 and adjust axis size only if necessary)
    if (propIndex > Lattice::AxisSize()) {
        weightCache = CoordTensor<float>(Lattice::Order() + 1, propIndex, INVALID_WEIGHT);
    } else {
        weightCache = CoordTensor<float>(Lattice::Order() + 1, Lattice::AxisSize(), INVALID_WEIGHT);
    }
    // Temporarily remove non-static modules from lattice
    for (const auto& mod : ModuleIdManager::FreeModules()) {
        Lattice::coordTensor[mod.coords] = FREE_SPACE;
    }
    // Populate weight tensor
    for (const auto& desiredModuleData : desiredState) {
        std::queue<SearchCoordProp> coordQueue;
        coordQueue.push({desiredModuleData.Coords(), 0, (desiredModuleData.Properties().AsInt())});
        while (!coordQueue.empty()) {
            //std::valarray<int> coords = coordQueue.front().coords;
            std::valarray<int> coordProps(Lattice::Order() + 1, 0);
            for (int i = 0; i < Lattice::Order(); i++) {
                coordProps[i] = coordQueue.front().coords[i];
            }
            coordProps[Lattice::Order()] = propConversionMap[coordQueue.front().propInt];
            const auto depth = coordQueue.front().depth;
            if (const auto weight = weightCache[coordProps]; depth < weight) {
                weightCache[coordProps] = depth;
            } else if (depth > weight) {
                coordQueue.pop();
                continue;
            }
            MoveOffsetPropertyEnqueueAdjacent(coordQueue, coordQueue.front());
            coordQueue.pop();
        }
    }
    // Restore non-static module to lattice
    for (const auto& mod : ModuleIdManager::FreeModules()) {
        Lattice::coordTensor[mod.coords] = mod.id;
    }
    // Print weight tensor
    std::cout << "Weight Cache:";
    for (int prop = 0; prop < propIndex; prop++) {
        for (int i = 0; i < weightCache.GetArrayInternal().size(); i++) {
            if (i % Lattice::AxisSize() == 0) std::cout << std::endl;
            if (weightCache.GetArrayInternal()[i] < 10) {
                std::cout << weightCache.GetArrayInternal()[i];
            } else if (weightCache.GetArrayInternal()[i] == INVALID_WEIGHT) {
                std::cout << "#";
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
}

float MoveOffsetPropertyHeuristicCache::operator[](const std::valarray<int> &coords, std::uint_fast64_t propInt) const {
    static std::valarray<int> coordProps(Lattice::Order() + 1, 0);
    for (int i = 0; i < Lattice::Order(); i++) {
        coordProps[i] = coords[i];
    }
    coordProps[Lattice::Order()] = propConversionMap[propInt];
    return weightCache[coordProps];
}

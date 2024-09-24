#include "HeuristicCache.h"
#include <queue>
#include "../lattice/Lattice.h"
#include "../moves/MoveManager.h"

constexpr float INVALID_WEIGHT = 999;

struct SearchCoord {
    std::valarray<int> coords;
    float depth;
};

void ChebyshevEnqueueAdjacent(std::queue<SearchCoord>& coordQueue, const SearchCoord& coordInfo) {
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

IHeuristicCache::IHeuristicCache(): weightCache(Lattice::Order(), Lattice::AxisSize(), INVALID_WEIGHT) {}

float IHeuristicCache::operator[](const std::valarray<int> &coords) const {
    return weightCache[coords];
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

void MoveOffsetEnqueueAdjacent(std::queue<SearchCoord>& coordQueue, const SearchCoord& coordInfo, const std::vector<std::valarray<int>> offsets) {
    std::vector<std::valarray<int>> adjCoords;
    adjCoords.push_back(coordInfo.coords);
    auto adjCoordsTemp = adjCoords;
    for (auto adj : adjCoordsTemp) {
        for (const auto& offset : offsets) {
            adj += offset;
            // should add obstacle detection here, to prevent "jumping" over static modules
            adjCoords.push_back(adj);
            adj -= offset;
        }
    }
    for (const auto& coord : adjCoords) {
        if (Lattice::coordTensor[coord] == OUT_OF_BOUNDS || Lattice::coordTensor[coord] >=
            ModuleIdManager::MinStaticID()) continue;
        coordQueue.push({coord, coordInfo.depth + 1});
    }
}

MoveOffsetHeuristicCache::MoveOffsetHeuristicCache(const std::set<ModuleData> &desiredState) {
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
            MoveOffsetEnqueueAdjacent(coordQueue, coordQueue.front(), MoveManager::_offsets);
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


#ifndef HEURISTICCACHE_H
#define HEURISTICCACHE_H
#include <queue>
#include <valarray>
#include <set>

#include "../coordtensor/CoordTensor.h"
#include "../modules/ModuleManager.h"

struct SearchCoord {
    std::valarray<int> coords;
    float depth;
};

class IHeuristicCache {
protected:
    CoordTensor<float> weightCache;
public:
    IHeuristicCache();

    virtual float operator[](const std::valarray<int>& coords) const;

    virtual ~IHeuristicCache() = default;
};

class ChebyshevHeuristicCache final : public IHeuristicCache {
private:
    static void ChebyshevEnqueueAdjacent(std::queue<SearchCoord>& coordQueue, const SearchCoord& coordInfo);
public:
    explicit ChebyshevHeuristicCache(const std::set<ModuleData>& desiredState);
};

class MoveOffsetHeuristicCache final : public IHeuristicCache {
private:
    static void MoveOffsetEnqueueAdjacent(std::queue<SearchCoord>& coordQueue, const SearchCoord& coordInfo);
public:
    explicit MoveOffsetHeuristicCache(const std::set<ModuleData>& desiredState);
};

struct SearchCoordProp final : public SearchCoord {
    std::uint_fast64_t propInt{};
};

// This cache ONLY works if all module properties remain the same throughout the search
class MoveOffsetPropertyHeuristicCache final : public IHeuristicCache {
private:
    static std::unordered_map<std::uint_fast64_t, int> propConversionMap;

    static void MoveOffsetPropertyEnqueueAdjacent(std::queue<SearchCoordProp>& coordPropQueue, const SearchCoordProp& coordPropInfo);
public:
    explicit MoveOffsetPropertyHeuristicCache(const std::set<ModuleData>& desiredState);

    float operator[](const std::valarray<int>& coords, std::uint_fast64_t propInt) const;
};

#endif //HEURISTICCACHE_H

#ifndef HEURISTICCACHE_H
#define HEURISTICCACHE_H
#include <valarray>
#include <set>

#include "../coordtensor/CoordTensor.h"
#include "../modules/ModuleManager.h"


class IHeuristicCache {
protected:
    CoordTensor<float> weightCache;
public:
    IHeuristicCache();

    virtual float operator[](const std::valarray<int>& coords) const;

    virtual ~IHeuristicCache() = default;
};

class ChebyshevHeuristicCache final : public IHeuristicCache {
public:
    explicit ChebyshevHeuristicCache(const std::set<ModuleData>& desiredState);
};

#endif //HEURISTICCACHE_H

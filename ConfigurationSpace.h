#include <vector>
#include "Lattice.h"

#ifndef MODULAR_ROBOTICS_CONFIGURATIONSPACE_H
#define MODULAR_ROBOTICS_CONFIGURATIONSPACE_H

class HashedState {
private:
    size_t seed;
public:
    HashedState();

    explicit HashedState(size_t seed);

    explicit HashedState(CoordTensor<bool> state);

    HashedState(const HashedState& other);

    size_t GetSeed() const;

    void HashLattice(const Lattice& lattice);

    void HashCoordTensor(const CoordTensor<bool>& state);

    bool operator==(const HashedState& other) const;
};

namespace std {
    template<>
    struct hash<HashedState> {
        size_t operator()(const HashedState& state) const;
    };
}

class Configuration {
private:
    Configuration* parent = nullptr;
    std::vector<Configuration*> next;
    CoordTensor<bool> _state;
    HashedState hash;
public:
    int depth = 0;
    explicit Configuration(CoordTensor<bool> state);

    ~Configuration();

    std::vector<CoordTensor<bool>> MakeAllMoves(Lattice& lattice);

    void AddEdge(Configuration* configuration);

    Configuration* GetParent();

    std::vector<Configuration*> GetNext();

    CoordTensor<bool> GetState();

    HashedState GetHash();

    void SetStateAndHash(const CoordTensor<bool>& state);

    void SetParent(Configuration* configuration);

    friend std::ostream& operator<<(std::ostream& out, const Configuration& config);
};

namespace ConfigurationSpace {
    std::vector<Configuration*> BFS(Configuration* start, Configuration* final, Lattice& lattice);

    std::vector<Configuration*> FindPath(Configuration* start, Configuration* final);
}

#endif //MODULAR_ROBOTICS_CONFIGURATIONSPACE_H

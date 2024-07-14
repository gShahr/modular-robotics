#ifndef HASHEDSTATE_H
#define HASHEDSTATE_H

#include <cstddef> // for size_t
#include "CoordTensor.h" // Assuming CoordTensor is defined in this header
#include "Lattice.h" // Assuming Lattice is defined in this header
#include <boost/functional/hash.hpp> // For boost::hash_range
#include <functional> // Required for std::hash specialization

class HashedState {
private:
    size_t seed;
public:
    HashedState();
    HashedState(size_t seed);
    HashedState(const CoordTensor<bool>& coordTensor);
    HashedState(const HashedState& other);

    size_t getSeed() const;
    void hashLattice(const Lattice& lattice);
    void hashCoordTensor(const CoordTensor<bool>& coordTensor);
    bool compareStates(const HashedState& other) const;
    bool compareLattice(const Lattice& Lattice1, const Lattice& Lattice2);
    bool operator==(const HashedState& other) const;
};

namespace std {
    template<>
    struct hash<HashedState> {
        std::size_t operator()(const HashedState& state) const {
            return std::hash<size_t>()(state.getSeed());
        }
    };
}

#endif // HASHEDSTATE_H
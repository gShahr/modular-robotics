#ifndef HASHEDSTATE_H
#define HASHEDSTATE_H

#include <cstddef> // for size_t
#include "CoordTensor.h" // Assuming CoordTensor is defined in this header
#include "Lattice.h" // Assuming Lattice is defined in this header
#include <boost/functional/hash.hpp> // For boost::hash_range

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

#endif // HASHEDSTATE_H
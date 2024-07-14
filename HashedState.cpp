#include "HashedState.h"

HashedState::HashedState() : seed(0) {}

HashedState::HashedState(size_t seed) : seed(seed) {}

HashedState::HashedState(const CoordTensor<bool>& coordTensor) {
    hashCoordTensor(coordTensor);
}

HashedState::HashedState(const HashedState& other) : seed(other.getSeed()) {}

size_t HashedState::getSeed() const { return seed; }

void HashedState::hashLattice(const Lattice& lattice) {
    seed = boost::hash_range(lattice.stateTensor.GetArrayInternal().begin(), lattice.stateTensor.GetArrayInternal().end());
}

void HashedState::hashCoordTensor(const CoordTensor<bool>& coordTensor) {
    seed = boost::hash_range(coordTensor.GetArrayInternal().begin(), coordTensor.GetArrayInternal().end());
}

bool HashedState::compareStates(const HashedState& other) const { return seed == other.getSeed(); }

bool HashedState::compareLattice(const Lattice& Lattice1, const Lattice& Lattice2) {
    // Implementation needed
    return false; // Placeholder return
}

bool HashedState::operator==(const HashedState& other) const {
    return seed == other.getSeed();
}
#include <iostream>
#include <cmath>
#include "debug_util.h"
#include "CoordTensor.h"

CoordTensor::CoordTensor(int order, int axisSize) {
    // Calculate number of elements in tensor
    int internalSize = (int) std::pow(axisSize, order);
    // Resize internal array to accommodate all elements
    _arrayInternal.resize(internalSize, -1);
    // Set size of 2nd order array
    internalSize = axisSize;
    switch (order) {
        case 3:
            // 2nd order array will need to be larger if tensor is 3rd order
            internalSize *= axisSize;
        case 2:
            // Initialize 2nd order array
            _arrayInternal2D.resize(internalSize);
            for (int i = 0; i < internalSize; i++) {
                _arrayInternal2D[i] = &(_arrayInternal[i * axisSize]);
            }
            if (internalSize == axisSize) {
                // If internalSize wasn't modified by case 3, then tensor is 2nd order
                IdAtInternal = &CoordTensor::IdAt2ndOrder;
                DEBUG("2nd order tensor created\n");
                break;
            }
            // Otherwise tensor is 3rd order
            // Initialize 3rd order array
            _arrayInternal3D.resize(axisSize);
            for (int i = 0; i < axisSize; i++) {
                _arrayInternal3D[i] = &(_arrayInternal2D[i * axisSize]);
            }
            IdAtInternal = &CoordTensor::IdAt3rdOrder;
            DEBUG("3rd order tensor created\n");
            break;
        default:
            // If the tensor is not 2nd or 3rd order then the
            // coordinate multiplier cache needs to be set up
            _axisMultipliers.resize(order);
            int multiplier = 1;
            for (int i = 0; i < order; i++) {
                _axisMultipliers[i] = multiplier;
                multiplier *= axisSize;
            }
            IdAtInternal = &CoordTensor::IdAtNthOrder;
            DEBUG("Tensor of order " << order << " created\n");
    }
#ifndef NDEBUG
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
#pragma ide diagnostic ignored "UnreachableCode"
    std::cout << "IdAtInternal Function: " << (
            IdAtInternal == &CoordTensor::IdAt3rdOrder ? "IdAt3rdOrder" :
            IdAtInternal == &CoordTensor::IdAt2ndOrder ? "IdAt2ndOrder" :
            IdAtInternal == &CoordTensor::IdAtNthOrder ? "IdAtNthOrder" :
            "Invalid Function!") << std::endl;
#pragma clang diagnostic pop
#endif
}

int& CoordTensor::GetIdDirect(int index) {
    return _arrayInternal[index];
}

int& CoordTensor::IdAt(const std::valarray<int>& coords) {
    // Told you calling member function pointers looks weird
    return (this->*IdAtInternal)(coords);
    // This would look even worse if IdAtInternal were called from outside,
    // it'd look like this: (tensor3D.*(tensor3D.IdAtInternal))({8, 4, 6}) <-- Not Good
}

int& CoordTensor::IdAt2ndOrder (const std::valarray<int>& coords) {
    return _arrayInternal2D[coords[0]][coords[1]];
}

int& CoordTensor::IdAt3rdOrder (const std::valarray<int>& coords) {
    return _arrayInternal3D[coords[0]][coords[1]][coords[2]];
}

int& CoordTensor::IdAtNthOrder (const std::valarray<int>& coords) {
    return _arrayInternal[(coords * _axisMultipliers).sum()];
}

int &CoordTensor::operator[](const std::valarray<int> &coords) {
    return (this->*IdAtInternal)(coords);
}

std::vector<int>::size_type CoordTensor::size() {
    return _arrayInternal.size();
}

#include <valarray>
#include <vector>

#include <iostream>
#include <cmath>
#include "debug_util.h"

#ifndef TENSORFINAL_COORDTENSOR_H
#define TENSORFINAL_COORDTENSOR_H

template <typename T>
class CoordTensor {
public:
    // Constructor, creates a tensor of specified order and axis length.
    // Order in this case is the amount of coordinates needed to
    // represent a point in space.
    // Axis size determines the length of each axis, an axis size of 10
    // would mean that only the integers 0-9 would be valid coordinates.
    CoordTensor(int order, int axisSize, const typename std::vector<T>::value_type& value);

    // Gets a reference to an ID directly from the internal array, this
    // is always faster than calling IdAtInternal but requires a pre-calculated
    // index in order to work.
    typename std::vector<T>::reference GetIdDirect(int index);

    // IdAt returns a reference to the module ID stored at the given
    // coordinates. An ID of -1 means no module is present.
    typename std::vector<T>::reference IdAt(const std::valarray<int>& coords);

    // Identical to IdAt but uses the subscript operator, mostly here to
    // make moving to CoordTensor easier.
    typename std::vector<T>::reference operator[](const std::valarray<int>& coords);

    // Get a const reference to the internal array
    [[nodiscard]]
    const std::vector<T>& GetArrayInternal() const;

    // Get a copy of order
    [[nodiscard]]
    int Order() const;

    // Get a copy of axisSize
    [[nodiscard]]
    int AxisSize() const;

    // Get a coordinate vector from an index
    [[nodiscard]]
    std::valarray<int> CoordsFromIndex(int index) const;

    // Comparison Operators
    bool operator==(const CoordTensor<T>& right);
    bool operator!=(const CoordTensor<T>& right);
private:
    int _order;
    // Axis size, useful for bounds checking
    int _axisSize;
    // Coordinate multiplier cache for tensors of order > 3
    std::valarray<int> _axisMultipliers;
    // Internal array for rapid conversion from index to coordinate
    std::vector<std::valarray<int>> _coordsInternal;
    // Internal array responsible for holding module IDs
    std::vector<T> _arrayInternal;
    // Internal array responsible for holding slices of _arrayInternal
    std::vector<T*> _arrayInternal2D;
    // Internal array responsible for holding slices of _arrayInternal2D
    std::vector<T**> _arrayInternal3D;
    // Did you know that calling member function pointers looks really weird?
    typename std::vector<T>::reference (CoordTensor::*IdAtInternal)(const std::valarray<int>& coords);
    // 2nd and 3rd order tensors benefit from specialized IdAt functions
    typename std::vector<T>::reference IdAt2ndOrder (const std::valarray<int>& coords);
    typename std::vector<T>::reference IdAt3rdOrder (const std::valarray<int>& coords);
    // Generalized IdAtInternal function
    typename std::vector<T>::reference IdAtNthOrder (const std::valarray<int>& coords);
};

template<typename T>
bool CoordTensor<T>::operator==(const CoordTensor<T>& right) {
    return _arrayInternal == right._arrayInternal;
}

template<typename T>
bool CoordTensor<T>::operator!=(const CoordTensor<T>& right) {
    return _arrayInternal != right._arrayInternal;
}

template<typename T>
int CoordTensor<T>::Order() const {
    return _order;
}

template<typename T>
int CoordTensor<T>::AxisSize() const {
    return _axisSize;
}

template<typename T>
std::valarray<int> CoordTensor<T>::CoordsFromIndex(int index) const {
    return _coordsInternal[index];
}

template <typename T>
CoordTensor<T>::CoordTensor(int order, int axisSize, const typename std::vector<T>::value_type& value) {
    _order = order;
    _axisSize = axisSize;
    // Calculate number of elements in tensor
    int internalSize = (int) std::pow(_axisSize, order);
    // Resize internal array to accommodate all elements
    _arrayInternal.resize(internalSize, value);
    // Setup Index -> Coordinate array
    _coordsInternal.resize(internalSize);
    std::valarray<int> coords;
    coords.resize(order, 0);
    for (int i = 0; i < internalSize; i++) {
        _coordsInternal[i] = coords;
        for (int j = 0; j < order; j++) {
            if (++coords[j] == axisSize) {
                coords[j] = 0;
            } else break;
        }
    }
    // Set size of 2nd order array
    internalSize = _axisSize;
    switch (order) {
        case 3:
            // 2nd order array will need to be larger if tensor is 3rd order
            internalSize *= _axisSize;
        case 2:
            // Initialize 2nd order array
            _arrayInternal2D.resize(internalSize);
            for (int i = 0; i < internalSize; i++) {
                _arrayInternal2D[i] = &(_arrayInternal[i * _axisSize]);
            }
            if (internalSize == _axisSize) {
                // If internalSize wasn't modified by case 3, then tensor is 2nd order
                IdAtInternal = &CoordTensor::IdAt2ndOrder;
                DEBUG("2nd order tensor created\n");
                break;
            }
            // Otherwise tensor is 3rd order
            // Initialize 3rd order array
            _arrayInternal3D.resize(_axisSize);
            for (int i = 0; i < _axisSize; i++) {
                _arrayInternal3D[i] = &(_arrayInternal2D[i * _axisSize]);
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
                multiplier *= _axisSize;
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

template<>
CoordTensor<bool>::CoordTensor(int order, int axisSize, const typename std::vector<bool>::value_type& value) {
    _order = order;
    _axisSize = axisSize;
    // Calculate number of elements in tensor
    int internalSize = (int) std::pow(_axisSize, order);
    // Resize internal array to accommodate all elements
    _arrayInternal.resize(internalSize, value);
    // Setup Index -> Coordinate array
    _coordsInternal.resize(internalSize);
    std::valarray<int> coords;
    coords.resize(order, 0);
    for (int i = 0; i < internalSize; i++) {
        _coordsInternal[i] = coords;
        for (int j = 0; j < order; j++) {
            if (++coords[j] == axisSize) {
                coords[j] = 0;
            } else break;
        }
    }
    // If the tensor is not 2nd or 3rd order then the
    // coordinate multiplier cache needs to be set up
    _axisMultipliers.resize(order);
    int multiplier = 1;
    for (int i = 0; i < order; i++) {
        _axisMultipliers[i] = multiplier;
        multiplier *= _axisSize;
    }
    IdAtInternal = &CoordTensor::IdAtNthOrder;
    DEBUG("Tensor of order " << order << " created\n");
#ifndef NDEBUG
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
#pragma ide diagnostic ignored "UnreachableCode"
    std::cout << "IdAtInternal Function: " << (
            IdAtInternal == &CoordTensor::IdAtNthOrder ? "IdAtNthOrder" :
            "Invalid Function!") << std::endl;
#pragma clang diagnostic pop
#endif
}

template <typename T>
typename std::vector<T>::reference CoordTensor<T>::GetIdDirect(int index) {
    return _arrayInternal[index];
}

template <typename T>
inline typename std::vector<T>::reference CoordTensor<T>::IdAt(const std::valarray<int>& coords) {
    // Told you calling member function pointers looks weird
    return (this->*IdAtInternal)(coords);
    // This would look even worse if IdAtInternal were called from outside,
    // it'd look like this: (tensor3D.*(tensor3D.IdAtInternal))({8, 4, 6}) <-- Not Good
}

template <typename T>
typename std::vector<T>::reference CoordTensor<T>::IdAt2ndOrder (const std::valarray<int>& coords) {
    return _arrayInternal2D[coords[1]][coords[0]];
}

template <typename T>
typename std::vector<T>::reference CoordTensor<T>::IdAt3rdOrder (const std::valarray<int>& coords) {
    return _arrayInternal3D[coords[2]][coords[1]][coords[0]];
}

template <typename T>
typename std::vector<T>::reference CoordTensor<T>::IdAtNthOrder (const std::valarray<int>& coords) {
    return _arrayInternal[(coords * _axisMultipliers).sum()];
}

template <typename T>
typename std::vector<T>::reference CoordTensor<T>::operator[](const std::valarray<int> &coords) {
    return (this->*IdAtInternal)(coords);
}

template <typename T>
const std::vector<T>& CoordTensor<T>::GetArrayInternal() const {
    return _arrayInternal;
}

// Would only work with C++20
/*template <typename T>
std::vector<T>::size_type CoordTensor<T>::size() const {
    return _arrayInternal.size();
}*/


#endif //TENSORFINAL_COORDTENSOR_H

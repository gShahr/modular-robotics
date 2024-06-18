#include <valarray>
#include <vector>
#ifndef TENSORFINAL_COORDTENSOR_H
#define TENSORFINAL_COORDTENSOR_H

class CoordTensor {
public:
    // Constructor, creates a tensor of specified order and axis length.
    // Order in this case is the amount of coordinates needed to
    // represent a point in space.
    // Axis size determines the length of each axis, an axis size of 10
    // would mean that only the integers 0-9 would be valid coordinates.
    CoordTensor(int order, int axisSize);

    // Gets a reference to an ID directly from the internal array, this
    // is always faster than calling IdAtInternal but requires a pre-calculated
    // index in order to work.
    int& GetIdDirect(int index);

    // IdAt returns a reference to the module ID stored at the given
    // coordinates. An ID of -1 means no module is present.
    int& IdAt(const std::valarray<int>& coords);

private:
    // Coordinate multiplier cache for tensors of order > 3
    std::valarray<int> _axisMultipliers;
    // Internal array responsible for holding module IDs
    std::vector<int> _arrayInternal;
    // Internal array responsible for holding slices of _arrayInternal
    std::vector<int*> _arrayInternal2D;
    // Internal array responsible for holding slices of _arrayInternal2D
    std::vector<int**> _arrayInternal3D;
    // Did you know that calling member function pointers looks really weird?
    int& (CoordTensor::*IdAtInternal)(const std::valarray<int>& coords);
    // 2nd and 3rd order tensors benefit from specialized IdAt functions
    int& IdAt2ndOrder (const std::valarray<int>& coords);
    int& IdAt3rdOrder (const std::valarray<int>& coords);
    // Generalized IdAtInternal function
    int& IdAtNthOrder (const std::valarray<int>& coords);
};

#endif //TENSORFINAL_COORDTENSOR_H

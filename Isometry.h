#include <vector>

#ifndef MODULAR_ROBOTICS_ISOMETRY_H
#define MODULAR_ROBOTICS_ISOMETRY_H

class ITransformable {
protected:
    int order = -1;
public:
    virtual void Rotate(int index) = 0;
    virtual void Reflect(int index) = 0;
    // May also add Translate(std::valarray offset) if it would be helpful
    [[nodiscard]]
    virtual ITransformable* MakeCopy() const = 0;

    friend class Isometry;
};

class Isometry {
public:
    static std::vector<ITransformable*> GenerateTransforms(ITransformable* initial);
};

#endif //MODULAR_ROBOTICS_ISOMETRY_H

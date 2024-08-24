#include <vector>

#ifndef MODULAR_ROBOTICS_ISOMETRY_H
#define MODULAR_ROBOTICS_ISOMETRY_H

class ITransformable {
protected:
    int order = -1;
public:
    virtual void Rotate(int a, int b) = 0;
    virtual void Reflect(int index) = 0;
    // May also add Translate(std::valarray offset) if it would be helpful
    [[nodiscard]]
    virtual ITransformable* MakeCopy() const = 0;

    virtual ~ITransformable() = default;

    friend class Isometry;
};

class Isometry {
private:
    static std::vector<ITransformable*> transformsToFree;
public:
    static std::vector<ITransformable*> GenerateTransforms(ITransformable* initial);

    static void CleanupTransforms();

    friend class MoveManager;
};

#endif //MODULAR_ROBOTICS_ISOMETRY_H

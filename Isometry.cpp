#include "Isometry.h"

std::vector<ITransformable*> Isometry::GenerateTransforms(ITransformable* initial) {
    // Set up working vector
    std::vector<ITransformable*> transforms = {initial};
    // Rotations
    for (int i = 0; i < initial->order; i++) {
        auto rotated = initial->MakeCopy();
        rotated->Rotate(i);
        transforms.push_back(rotated);
    }
    // Reflections
    for (int i = 0; i < initial->order; i++) {
        auto forms = transforms;
        for (auto form : forms) {
            auto reflected = form->MakeCopy();
            reflected->Reflect(i);
            transforms.push_back(reflected);
        }
    }
    // Done
    return transforms;
}
#include "Isometry.h"

std::vector<ITransformable*> Isometry::transformsToFree;

// If you are trying to refactor this good luck
std::vector<ITransformable*> Isometry::GenerateTransforms(ITransformable* initial) {
    // Set up working vector
    std::vector<ITransformable*> transforms = {initial};
    // Rotations
    for (int i = 0; i < initial->order - 1; i++) {
        auto forms = transforms;
        for (int j = i + 1; j < initial->order; j++) {
            for (auto form : forms) {
                auto rotated = form->MakeCopy();
                rotated->Rotate(i, j);
                transforms.push_back(rotated);
                transformsToFree.push_back(rotated);
            }
        }
    }
    // Reflections
    for (int i = 0; i < initial->order; i++) {
        auto forms = transforms;
        for (auto form : forms) {
            auto reflected = form->MakeCopy();
            reflected->Reflect(i);
            transforms.push_back(reflected);
            transformsToFree.push_back(reflected);
        }
    }
    // Done
    return transforms;
}

void Isometry::CleanupTransforms() {
    for (auto transform : transformsToFree) {
        delete transform;
    }
}
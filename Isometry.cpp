#include "Isometry.h"

std::vector<ITransformable*> Isometry::transformsToFree;

// If you are trying to refactor this good luck
std::vector<ITransformable*> Isometry::GenerateTransforms(ITransformable* initial) {
    // Set up working vector
    std::vector<ITransformable*> transforms = {initial};
    // Reflection
    auto reflected = initial->MakeCopy();
    reflected->Reflect(0);
    transforms.push_back(reflected);
    transformsToFree.push_back(reflected);
    // Rotations
    for (int i = 0; i < initial->order; i++) {
        for (int j = i + 1; j < initial->order; j++) {
            auto forms = transforms;
            for (auto form : forms) {
                auto rotated = form->MakeCopy();
                rotated->Rotate(i, j);
                // Reflections of Rotations
                for (int k = 0; k < initial->order; k++) {
                    auto reflectedRotate = rotated->MakeCopy();
                    reflectedRotate->Reflect(k);
                    transforms.push_back(reflectedRotate);
                    transformsToFree.push_back(reflectedRotate);
                }
                transforms.push_back(rotated);
                transformsToFree.push_back(rotated);
            }
        }
    }
    /*// Rotations
    for (int i = 0; i < initial->order; i++) {
        for (int j = i + 1; j < initial->order; j++) {
            auto rotated = initial->MakeCopy();
            rotated->Rotate(i, j);
            transforms.push_back(rotated);
            transformsToFree.push_back(rotated);
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
    }*/
    // Done
    return transforms;
}

void Isometry::CleanupTransforms() {
    for (auto transform : transformsToFree) {
        delete transform;
    }
}
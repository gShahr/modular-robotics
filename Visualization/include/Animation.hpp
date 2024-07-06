#ifndef ANIMATION_H
#define ANIMATION_H

#include <iostream>
#include "glm/glm.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

class Animation
{
public:
    Animation(glm::vec3 AnchorDirection, glm::vec3 DeltaPos, bool sliding);
    bool sliding;
    glm::vec3 AnchorDirection;
    glm::vec3 DeltaPos;
    float MaxAngle;
    glm::vec3 PreTranslation;
    glm::vec3 RotationAxis;
};

#endif

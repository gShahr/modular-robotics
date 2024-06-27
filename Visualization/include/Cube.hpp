#ifndef CUBE_H
#define CUBE_H

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext.hpp"
#include "glad/glad.h"
#include "glfw3.h"
#include "Animation.hpp"

unsigned int _createCubeVAO();
extern unsigned int transformLoc, modelLoc, surfaceNormalLoc; // Assigned whenever a shader is loaded
extern float deltaTime, ANIM_SPEED;
extern bool ANIMATE;

class Cube
{
public:
    Cube(float x, float y, float z);
    void setPos(float x, float y, float z);
    void draw();
    void startAnimation(glm::vec3 AnchorDirection, glm::vec3 DeltaPos);
    glm::mat4 processAnimation(bool *animFinished);
private:
    float x, y, z;
    Animation *anim;
    float animProgress;
};

#endif

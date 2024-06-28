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
    Cube(glm::vec3 pos);
    void setPos(glm::vec3 newPos);
    void getPos();
    void draw();
    void startAnimation(glm::vec3 AnchorDirection, glm::vec3 DeltaPos);
    void stopAnimation();
    glm::mat4 processAnimation(bool *animFinished);
private:
    glm::vec3 pos;
    Animation *anim;
    float animProgress;
};

#endif

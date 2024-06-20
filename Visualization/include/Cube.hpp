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

unsigned int _createCubeVAO();
extern unsigned int modelLoc, surfaceNormalLoc; // Assigned whenever a shader is loaded

class Cube
{
public:
    Cube(float x, float y, float z);
    void setPos(float x, float y, float z);
    void draw();
    void setRotation(float angle);
    void setRotation(float angle, glm::vec3 rotAxis);
    void setPreTranslation(glm::vec3 pt);
private:
    float x, y, z;
    float angle;
    glm::vec3 rotAxis;
    glm::vec3 pt;
};

#endif

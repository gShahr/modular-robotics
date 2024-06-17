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
extern unsigned int modelLoc; // Assigned whenever a shader is loaded

class Cube
{
public:
    Cube(float x, float y, float z);
    void setPos(float x, float y, float z);
    void draw();
private:
    float x, y, z;
};

#endif

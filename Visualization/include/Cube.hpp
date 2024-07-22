#ifndef CUBE_H
#define CUBE_H

#include <iostream>
#include <cmath>
#include <map>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext.hpp"
#include "glad/glad.h"
#include "glfw3.h"
#include "Move.hpp"
#include "Shader.hpp"

unsigned int _createCubeVAO();
extern Shader* glob_shader;
extern float glob_deltaTime, glob_animSpeed;
extern bool glob_animate;

class Cube
{
public:
    Cube(int id, int x, int y, int z);
    void setPos(int x, int y, int z);
    void setScale(int scale);
    void setColor(int r, int g, int b);
    void draw();
    void startAnimation(bool* markWhenAnimFinished, Move* move);
    void stopAnimation();
    glm::mat4 processAnimation();
    glm::vec3 pos;
    Move* move;
    bool* markWhenAnimFinished;
    int id;
private:
    float animProgress;
    glm::vec3 color;
    glm::vec3 scale;
    glm::mat4 rotation;
};

extern std::unordered_map<int, Cube*> glob_objects; // This is ugly, sorry

#endif

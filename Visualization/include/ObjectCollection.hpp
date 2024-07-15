#ifndef OBJECTCOLLECTION_H
#define OBJECTCOLLECTION_H

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Shader.hpp"
#include "Cube.hpp"
#include "Camera.hpp"

extern unsigned int glob_timeLoc, glob_transformLoc, glob_viewLoc, glob_projLoc; // Assigned whenever a shader is loaded
extern glm::mat4 glob_transform;
extern Camera camera;
extern float glob_lastFrame;

class ObjectCollection
{
public:
    ObjectCollection(Shader *shader, unsigned int VAO, int textureID = -1);
    void drawAll();
    void addObj(Cube *cube);
private:
    Shader *shader;
    unsigned int VAO;
    std::vector<Cube*> objects;
    int numObjs, textureID;
};

#endif

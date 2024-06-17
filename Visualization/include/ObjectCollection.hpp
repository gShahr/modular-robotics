#ifndef OBJECTCOLLECTION_H
#define OBJECTCOLLECTION_H

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Shader.hpp"
#include "Cube.hpp"

extern unsigned int transformLoc, viewLoc, projLoc; // Assigned whenever a shader is loaded
extern glm::mat4 viewmat, projmat, transform; // main()

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

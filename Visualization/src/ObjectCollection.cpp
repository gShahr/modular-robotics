#include "ObjectCollection.hpp"

ObjectCollection::ObjectCollection(Shader *shader, unsigned int VAO, int textureID) {
    this->shader = shader;
    this->VAO = VAO;
    this->objects = std::vector<Cube*>();
    this->numObjs = 0;
    this->textureID = textureID;
}

void ObjectCollection::drawAll() {
    int i; 
    this->shader->use();

    //std::cout << glm::to_string(viewmat) << std::endl;
    
    glUniform1f(glob_timeLoc, glob_lastFrame);

    glUniformMatrix4fv(glob_viewLoc, 1, GL_FALSE, glm::value_ptr(glob_viewmat));
    glUniformMatrix4fv(glob_projLoc, 1, GL_FALSE, glm::value_ptr(glob_projmat));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->textureID);
    glBindVertexArray(this->VAO);

    // std::cout << glm::to_string(viewmat) << std::endl;
    // std::cout << modelLoc << " | " << transformLoc << " | " << viewLoc << " | " << projLoc << std::endl;
    for (i = 0; i < this->numObjs; i++) {
        this->objects[i]->draw();
    }

    glBindVertexArray(0);
}

void ObjectCollection::addObj(Cube *cube) {
    (this->objects).push_back(cube);
    this->numObjs++;
}

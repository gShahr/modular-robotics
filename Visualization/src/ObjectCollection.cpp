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

    glUniform1f(glob_shader->timeLoc, glob_lastFrame);

    glUniformMatrix4fv(glob_shader->viewLoc, 1, GL_FALSE, glm::value_ptr(camera.getViewMat()));
    glUniformMatrix4fv(glob_shader->projLoc, 1, GL_FALSE, glm::value_ptr(camera.getProjMat()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->textureID);
    glBindVertexArray(this->VAO);

    for (i = 0; i < this->numObjs; i++) {
        this->objects[i]->draw();
    }

    glBindVertexArray(0);
}

void ObjectCollection::addObj(Cube *cube) {
    (this->objects).push_back(cube);
    this->numObjs++;
}

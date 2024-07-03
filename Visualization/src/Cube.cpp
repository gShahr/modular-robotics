#include "Cube.hpp"

float _cubeVertices[] = { 
    //    Coords            Tex Coord
    // Back face:
     0.5f, -0.5f, -0.5f,    0.0f, 0.0f, // Bottom left    0
    -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, // Bottom right   1
    -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, // Top right      2
     0.5f,  0.5f, -0.5f,    0.0f, 1.0f, // Top left       3
    // Front face:
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, // Bottom left    4
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, // Bottom right   5
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, // Top right      6
    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, // Top left       7
    // Left face:
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, // Bottom left    8
    -0.5f, -0.5f,  0.5f,    1.0f, 0.0f, // Bottom right   9
    -0.5f,  0.5f,  0.5f,    1.0f, 1.0f, // Top right      10
    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, // Top left       11
    // Right face:
     0.5f, -0.5f,  0.5f,    0.0f, 0.0f, // Bottom left    12
     0.5f, -0.5f, -0.5f,    1.0f, 0.0f, // Bottom right   13
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, // Top right      14
     0.5f,  0.5f,  0.5f,    0.0f, 1.0f, // Top left       15
    // Bottom face:                   
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, // Bottom left    16
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, // Bottom right   17
     0.5f, -0.5f, -0.5f,    1.0f, 1.0f, // Top right      18
    -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, // Top left       19
    // Top face:                      
    -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, // Bottom left    20
     0.5f,  0.5f, -0.5f,    1.0f, 0.0f, // Bottom right   21
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, // Top right      22
    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, // Top left       23
};

unsigned int _cubeIndices[] = {  // For the Element Buffer Object
    0, 1, 2,    0, 2, 3,    // Back face
    4, 5, 6,    4, 6, 7,    // Front face
    8, 9, 10,   8, 10, 11,  // Left face
    12, 13, 14, 12, 14, 15, // Right face
    16, 17, 18, 16, 18, 19, // Bottom face
    20, 21, 22, 20, 22, 23  // Top face
};

glm::vec3 _cubeSurfaceNorms[] = {
    glm::vec3(0.0, 0.0, -1.0), 
    glm::vec3(0.0, 0.0, 1.0), 
    glm::vec3(-1.0, 0.0, 0.0), 
    glm::vec3(1.0, 0.0, 0.0), 
    glm::vec3(0.0, -1.0, 0.0), 
    glm::vec3(0.0, 1.0, 0.0)
};

unsigned int _createCubeVAO() {
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_cubeVertices), _cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);     // Bind the EBO to the active GL_ELEMENT_ARRAY_BUFFER
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_cubeIndices), _cubeIndices, GL_STATIC_DRAW); // Cp index data into EBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // Configure vertex attribs
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    return VAO;
}

void Cube::setPos(int x, int y, int z) {
    this->pos = glm::vec3(x, y, z);
}

void Cube::setScale(int scale) {
    this->scale = glm::vec3((float)scale / 100.0f);
}

void Cube::setColor(int r, int g, int b) {
    this->color = glm::vec3((float)r/255.0f, (float)g/255.0f, (float)b/255.0f);
}

Cube::Cube(int id, int x, int y, int z) {
    this->id = id;
    this->setPos(x, y, z);
    this->anim = NULL;
    this->scale = glm::vec3(0.9f);
    this->color = glm::vec3(1.0f);
    this->rotation = glm::mat4(1.0f);
    gObjects.insert(std::pair<int, Cube*>(id, this));
}

void Cube::startAnimation(bool* markWhenAnimFinished, glm::vec3 AnchorDirection, glm::vec3 DeltaPos) {
    this->markWhenAnimFinished = markWhenAnimFinished;
    this->anim = new Animation(AnchorDirection, DeltaPos);
    this->animProgress = 0.0f;
}

// Interpolation function used for animation progress: Should map [0.0, 1.0] -> [0.0, 1.0]
inline float _animInterp(float pct) {
    //return pct < 0.5 ? 4 * pct * pct * pct : 1 - std::pow(-2 * pct + 2, 3) / 2; // Cubic ease in-out
    return pct < 0.5 ? 2 * pct * pct : 1 - std::pow(-2 * pct + 2, 2) / 2; // Quadratic ease in-out
    //return pct;
}

glm::mat4 Cube::processAnimation() {
    glm::mat4 transform = glm::mat4(1.0f);

    // increment animation progress
    if (ANIMATE) {
        this->animProgress += (ANIM_SPEED * deltaTime);
    }

    if (animProgress > 1.0f) { // Animation finished
        this->rotation = glm::rotate(glm::mat4(1.0f), glm::radians(this->anim->MaxAngle), this->anim->RotationAxis) * this->rotation;

        transform = glm::translate(transform, this->anim->PreTranslation);
        transform = glm::rotate(transform, glm::radians(this->anim->MaxAngle), this->anim->RotationAxis);
        transform = glm::translate(transform, -(this->anim->PreTranslation));


        glm::vec3 newPos = this->pos + this->anim->DeltaPos;
        this->setPos(newPos[0], newPos[1], newPos[2]);
        this->animProgress = 0.0f;

        delete this->anim;
        this->anim = NULL;

        *(this->markWhenAnimFinished) = true; 
        this->markWhenAnimFinished = NULL;
    } else {
        // calculate rotation angle based on progress
        float angle = _animInterp(this->animProgress) * this->anim->MaxAngle;

        // construct transform matrix
        transform = glm::translate(transform, this->anim->PreTranslation);
        transform = glm::rotate(transform, glm::radians(angle), this->anim->RotationAxis);
        transform = glm::translate(transform, -(this->anim->PreTranslation));
    }

    return transform;
}

void Cube::draw() {
    // If an animation finishes, it may update the position. So, calculate modelmatrix now; animation will handle its own relevant translations/offsets
    glm::mat4 modelmat = glm::translate(glm::mat4(1.0f), glm::vec3(this->pos));

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::scale(transform, this->scale);
    transform = this->rotation * transform;
    if (this->anim) {
        transform = this->processAnimation() * transform;
    };

    glUniform3fv(colorLoc, 1, glm::value_ptr(this->color));
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelmat));
    if (surfaceNormalLoc >= 0) {
        for (int i = 0; i < 6; i++) {
            glUniform3fv(surfaceNormalLoc, 1, glm::value_ptr(_cubeSurfaceNorms[i]));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)((6 * i * sizeof(GLuint))));
        }
    } else { 
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
}

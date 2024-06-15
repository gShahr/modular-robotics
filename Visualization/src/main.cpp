#include <iostream>
#include <stdio.h>
#include <math.h>
#include "glad/glad.h"
#include "glfw3.h"
#include "Shader.hpp"
#include "stb_image.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

const char *vertexShaderPath = "resources/shaders/vshader.glsl";
const char *fragmentShaderPath = "resources/shaders/fshader.glsl";

float cubeVertices[] = { 
    //    Coords              Vertex color         Tex Coord
    // Back face:
    -0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 0.0f, // Bottom left    0
     0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 0.0f, // Bottom right   1
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 1.0f, // Top right      2
    -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 1.0f, // Top left       3
    // Front face:
    -0.5f, -0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 0.0f, // Bottom left    4
     0.5f, -0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 0.0f, // Bottom right   5
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 1.0f, // Top right      6
    -0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 1.0f, // Top left       7
    // Left face:
    -0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 0.0f, // Bottom left    8
    -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 1.0f, // Bottom right   9
    -0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 1.0f, // Top right      10
    -0.5f, -0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 0.0f, // Top left       11
    // Right face:
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 0.0f, // Bottom left    12
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 1.0f, // Bottom right   13
     0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 1.0f, // Top right      14
     0.5f, -0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 0.0f, // Top left       15
    // Bottom face:
    -0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 1.0f, // Bottom left    16
     0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 1.0f, // Bottom right   17
     0.5f, -0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 0.0f, // Top right      18
    -0.5f, -0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 0.0f, // Top left       19
    // Top face:
    -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 1.0f, // Bottom left    20
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 1.0f, // Bottom right   21
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     1.0f, 0.0f, // Top right      22
    -0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,     0.0f, 0.0f, // Top left       23
};

unsigned int indices[] = {  // For the Element Buffer Object
    0, 1, 2,    0, 2, 3,    // Back face
    4, 5, 6,    4, 6, 7,    // Front face
    8, 9, 10,   8, 10, 11,  // Left face
    12, 13, 14, 12, 14, 15, // Right face
    16, 17, 18, 16, 18, 19, // Bottom face
    20, 21, 22, 20, 22, 23  // Top face
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main(int argc, char** argv) {
    std::cout << "Hello, world" << std::endl;
    
    // Initialize GLFW and configure it to use version 3.3 with OGL Core Profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create the Window object and set it to the current Context
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Invoke GLAD to load addresses of OpenGL functions in the GPU drivers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shader = Shader(vertexShaderPath, fragmentShaderPath);

    // Establish the Viewport
    glViewport(0, 0, 800, 600);
    // Enable z-buffer depth testing
    glEnable(GL_DEPTH_TEST);

    // Register the window-resizing callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    stbi_set_flip_vertically_on_load(true);
    int twidth, theight, tchan;
    unsigned char *tdata = stbi_load("resources/textures/face_debug.png", &twidth, &theight, &tchan, 0);
    unsigned int texture;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Texture wrapping: repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmaps for distant textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Use linear filtering for close objects
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(tdata);

    // Create a Vertex Array Object to store our vertex attribute configuration, and bind it now
    // Create a Vertex Buffer Object on the GPU to store Vertex data, and bind it now
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);                     // Create a VAO
    glGenBuffers(1, &VBO);                          // Create a VBO
    glGenBuffers(1, &EBO);                          // Create an EBO
    glBindVertexArray(VAO);                         // Bind the VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);             // Bind the VBO to the active GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW); // Cp vertex data into VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);     // Bind the EBO to the active GL_ELEMENT_ARRAY_BUFFER
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Cp index data into EBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Configure vertex attribs
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glm::mat4 modelmat = glm::mat4(1.0f);

    glm::mat4 viewmat = glm::mat4(1.0f);
    viewmat = glm::translate(viewmat, glm::vec3(0.0f, 0.0f, -3.0f));

    glm::mat4 projmat;
    projmat = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
    //transform = glm::translate(transform, glm::vec3(-0.5f, -0.5f, 0.0f));

    unsigned int transformLoc, modelLoc, viewLoc, projLoc;
    while(!glfwWindowShouldClose(window)) {
        // -- Input ---
        processInput(window);

        modelmat = glm::rotate(modelmat, 0.01f, glm::vec3(0.5f, 1.0f, 0.0f));
        
        // -- Rendering --
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        transformLoc = glGetUniformLocation(shader.ID, "transform");
        modelLoc = glGetUniformLocation(shader.ID, "modelmat");
        viewLoc = glGetUniformLocation(shader.ID, "viewmat");
        projLoc = glGetUniformLocation(shader.ID, "projmat");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelmat));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewmat));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projmat));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    std::cout << "Goodbye, world\n";
    return 0;
}

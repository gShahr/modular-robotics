#include <iostream>
#include <stdio.h>
#include <math.h>
#include "glad/glad.h"
#include "glfw3.h"
#include "Shader.hpp"

const char *vertexShaderPath = "resources/shaders/vshader.glsl";
const char *fragmentShaderPath = "resources/shaders/fshader.glsl";

float vertices[] = {
    0.0f, 0.9f, 0.0f,   1.0f, 0.0f, 0.0f, // top
    -0.7f, 0.2f, 0.0f,  0.0f, 0.0f, 0.0f, // top left
    -0.3f, -0.9f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
    0.3f, -0.9f, 0.0f,  0.0f, 0.0f, 1.0f, // bottom right
    0.7f, 0.2f, 0.0f,   1.0f, 1.0f, 1.0f  // top right
};

unsigned int indices[] = { // For our Element Buffer Object
    0, 1, 2,    // first triangle
    1, 2, 3,    // second triangle
    2, 3, 4,    // third triangle
    3, 4, 0,    // fourth triangle
    4, 0, 1,    // fifth triangle
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

    // Register the window-resizing callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Create a Vertex Array Object to store our vertex attribute configuration, and bind it now
    // Create a Vertex Buffer Object on the GPU to store Vertex data, and bind it now
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);                     // Create a VAO
    glGenBuffers(1, &VBO);                          // Create a VBO
    glGenBuffers(1, &EBO);                          // Create an EBO
    glBindVertexArray(VAO);                         // Bind the VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);             // Bind the VBO to the active GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Cp vertex data into VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);     // Bind the EBO to the active GL_ELEMENT_ARRAY_BUFFER
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Cp index data into EBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // Configure vertex attribs
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // std::cout << sizeof(vertices) << std::endl;

    while(!glfwWindowShouldClose(window)) {
        // -- Input ---
        processInput(window);
        
        // -- Rendering --
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 15, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    std::cout << "Goodbye, world\n";
    return 0;
}

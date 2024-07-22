/* Setup functions and miscellanous utility functions */
#include <iostream>

#include "glad/glad.h"
#include "glfw3.h"
#include "stb_image.h"

extern void framebuffer_size_callback(GLFWwindow* window, int width, int height);           // userinput.cpp
extern void cursormove_callback(GLFWwindow* window, double xpos, double ypos);              // userinput.cpp
extern void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);    // userinput.cpp
extern void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);      // userinput.cpp
extern void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);  // userinput.cpp
extern float glob_resolution[2];                                                            // main.cpp

int loadTexture(const char *texturePath) {
    stbi_set_flip_vertically_on_load(true);
    int twidth, theight, tchan;
    unsigned char *tdata = stbi_load(texturePath, &twidth, &theight, &tchan, 0);
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
    return texture;
}

GLFWwindow* createWindowAndContext() {
    // Initialize GLFW and configure it to use version 3.3 with OGL Core Profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the Window object and set it to the current Context
    GLFWwindow* window = glfwCreateWindow(glob_resolution[0], glob_resolution[1], "Modular Robotics", NULL, NULL);
    if (window == NULL) { throw std::runtime_error("Failed to create GLFW window"); }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Invoke GLAD to load addresses of OpenGL functions in the GPU drivers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { throw std::runtime_error("Failed to initialize GLAD"); }

    return window;
}

void registerWindowCallbacks(GLFWwindow* window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursormove_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);
}

void setupGl(GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


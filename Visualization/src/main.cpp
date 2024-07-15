#include <iostream>
#include <stdio.h>
#include <math.h>
#include <algorithm>            // clamp
#include <map>
#include <unordered_map>
#include "glad/glad.h"
#include "glfw3.h"
#include "stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Cube.hpp"
#include "ObjectCollection.hpp"
#include "Scenario.hpp"
#include "Camera.hpp"

#define AUTO_ROTATE 0

std::unordered_map<int, Cube*> glob_objects;    // Hashmap of all <ID, object>. Global variable

float glob_resolution[2] = {1280.0f, 720.0f};        // Screen attributes
float glob_aspectRatio = glob_resolution[0] / glob_resolution[1];

const char *vertexShaderPath = "resources/shaders/vshader.glsl";    // Resource paths
const char *fragmentShaderPath = "resources/shaders/fshader.glsl";
const char *texturePath = "resources/textures/face_debug.png";

float glob_deltaTime = 0.0f;                    // Frame-time info
float glob_lastFrame = 0.0f;
float glob_animSpeed = 2.0f;                    // Animation attributes
bool glob_animate = false;

Camera camera = Camera();

// Forward declerations -- definitions for these are in userinput.cpp
extern void framebuffer_size_callback(GLFWwindow* window, int width, int height);
extern void cursormove_callback(GLFWwindow* window, double xpos, double ypos);
extern void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
extern void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
extern void processInput(GLFWwindow *window);

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

int main(int argc, char** argv) {
    std::cout << "Hello, world" << std::endl;

    // Initialize GLFW and configure it to use version 3.3 with OGL Core Profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create the Window object and set it to the current Context
    GLFWwindow* window = glfwCreateWindow(glob_resolution[0], glob_resolution[1], "Modular Robotics", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    // Register window callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursormove_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);

    // Invoke GLAD to load addresses of OpenGL functions in the GPU drivers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Establish the Viewport
    glViewport(0, 0, glob_resolution[0], glob_resolution[1]);
    // Enable z-buffer depth testing and transparency
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader = Shader(vertexShaderPath, fragmentShaderPath);
    int texture = loadTexture(texturePath);
    unsigned int VAO = _createCubeVAO();
    
    std::string _scenfile;
    if (!argv[1]) {
        _scenfile.append("Scenarios/3d2rMeta.scen");
    } else {
        _scenfile.append("Scenarios/").append(argv[1]).append(".scen");
    }
    Scenario scenario = Scenario(_scenfile.c_str());
   
    ObjectCollection* scenCubes = scenario.toObjectCollection(&shader, VAO, texture);
    MoveSequence* scenMoveSeq = scenario.toMoveSequence();

    bool readyForNewAnim = true;
    bool forward = true;

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        glob_deltaTime = currentFrame - glob_lastFrame;
        glob_lastFrame = currentFrame;

        processInput(window);

#if AUTO_ROTATE < 1
        camera.setPos(camera.getPos() + camera.getSpeed()[2] * camera.getDir() * glob_deltaTime);
        camera.setPos(camera.getPos() + camera.getSpeed()[0] * glm::cross(camera.getDir(), camera.getUp()) * glob_deltaTime);
        camera.setPos(camera.getPos() + camera.getSpeed()[1] * camera.getUp() * glob_deltaTime);
        camera.calcViewMat();
#else
        camera.setPos(glm::rotate(camera.getPos(), glm::radians(15.0f * glob_deltaTime), glm::vec3(0.0, 1.0, 0.0)));
        camera.calcViewMat(glm::vec3(0.0f));
#endif

        // -- Rendering --
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (readyForNewAnim) {
            Move* move;
            if (forward) { move = scenMoveSeq->pop(); }
            else { move = scenMoveSeq->undo(); }

            Cube* mover = glob_objects.at(move->moverId);

            mover->startAnimation(&readyForNewAnim, move);
            readyForNewAnim = false;
            if ((scenMoveSeq->currentMove == 0) || (scenMoveSeq->remainingMoves == 0)) { 
                forward = !forward; 
            }
        }

        scenCubes->drawAll();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
    std::cout << "Goodbye, world\n";
    return 0;
}

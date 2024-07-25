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
bool glob_animateRequest = false;               //  When the current animation finishes (if any), should another be fetched?
bool glob_animateAuto = false;                  //  Is the scene in auto-animate mode? (will automa
bool glob_animateForward = true;                //  Direction to fetch animations

Camera camera = Camera();

// Forward declarations -- definitions for these are in userinput.cpp
extern void processInput(GLFWwindow *window);

// Forward declarations -- definitions for these are in setuputils.cpp
extern int loadTexture(const char *texturePath);
extern GLFWwindow* createWindowAndContext();
extern void registerWindowCallbacks(GLFWwindow* window);
extern void setupGl(GLFWwindow* window);

// TODO clean this up, comment it, and bulletproof it
Cube* raymarch(glm::vec3 pos, glm::vec3 dir) {
    float dist;
    float mindist = 100.0f;
    int i;
    Cube* cube;

    for (i = 20; (i > 0) && (mindist > 0.0f); i--) {
        mindist = 100.0f;
        for (std::pair<int, Cube*> pair : glob_objects) {
            cube = pair.second;
            dist = cube->distanceTo(pos);
            mindist = glm::min(dist, mindist);
            if (mindist == 0.0f) { std::cout << "Clicked on cube ID " << cube->id << std::endl; return cube; }
        }

        pos += glm::max(0.1f, mindist * 0.99f) * dir;
    }
    std::cout << "No cube at click location" << std::endl;
    return NULL;
}

int main(int argc, char** argv) {
    // Establishes a Window, creates an OpenGL context, and invokes GLAD
    GLFWwindow* window = createWindowAndContext(); // setuputils.cpp
    
    // Register window callbacks for user-input
    registerWindowCallbacks(window); // setuputils.cpp

    // Establish the Viewport and set GL settings (depth test/z-buffer, transparency, etc)
    setupGl(window); // setuputils.cpp

    // Load shaders and textures
    Shader shader = Shader(vertexShaderPath, fragmentShaderPath);
    int texture = loadTexture(texturePath); // setuputils.cpp

    // Create a Vertex Attribute Object for modules/cubes (collection of vertices and associated info on how to interpret them for GL)
    unsigned int VAO = _createCubeVAO();
    
    // Load the Scenario file
    std::string _scenfile;
    if (!argv[1]) { _scenfile.append("Scenarios/3d2rMeta.scen"); } 
    else { _scenfile.append("Scenarios/").append(argv[1]).append(".scen"); }
    Scenario scenario = Scenario(_scenfile.c_str());
   
    // Extract the modules and moves from the Scenario file
    ObjectCollection* scenCubes = scenario.toObjectCollection(&shader, VAO, texture);
    MoveSequence* scenMoveSeq = scenario.toMoveSequence();

    // Initialize for our main loop: we are ready for the next animation
    bool readyForNewAnim = true;

    // Main loop -- process input, update camera, handle animations, and render
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

        if (readyForNewAnim && glob_animateRequest) {
            Move* move;
            glob_animateRequest = glob_animateAuto;  // Set global variable: reset the request-anim flag, unless auto-animating

            if (glob_animateForward) { move = scenMoveSeq->pop(); }
            else { move = scenMoveSeq->undo(); }

            if (move) {
                Cube* mover = glob_objects.at(move->moverId);

                mover->startAnimation(&readyForNewAnim, move);
                readyForNewAnim = false;
            } else { glob_animateRequest = false; }
        }

        // -- Rendering --
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scenCubes->drawAll();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
    std::cout << "Goodbye, world\n";
    return 0;
}

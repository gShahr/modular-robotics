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

// Forward declarations -- definitions for these are in userinput.cpp
extern void processInput(GLFWwindow *window);

// Forward declarations -- definitions for these are in setuputils.cpp
extern int loadTexture(const char *texturePath);
extern GLFWwindow* createWindowAndContext();
extern void registerWindowCallbacks(GLFWwindow* window);
extern void setupGl();

int main(int argc, char** argv) {
    // Establishes a Window, creates an OpenGL context, and invokes GLAD
    GLFWwindow* window = createWindowAndContext(); // setuputils.cpp
    
    // Register window callbacks for user-input
    registerWindowCallbacks(window); // setuputils.cpp

    // Establish the Viewport and set GL settings (depth test/z-buffer, transparency, etc)
    setupGl(); // setuputils.cpp

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

    // Initialize for our main loop: we are ready for the next animation, and we want to move forward through the Scenario
    bool readyForNewAnim = true;
    bool forward = true;

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

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

std::unordered_map<int, Cube*> glob_objects;        // Hashmap of all <ID, object>. Global variable
glm::mat4 glob_viewmat, glob_projmat;           // View and Projection matrices

float resolution[2] = {1280.0f, 720.0f};        // Screen attributes
float glob_aspectRatio = resolution[0] / resolution[1];

const char *vertexShaderPath = "resources/shaders/vshader.glsl";    // Resource paths
const char *fragmentShaderPath = "resources/shaders/fshader.glsl";
const char *texturePath = "resources/textures/face_debug.png";

float glob_deltaTime = 0.0f;                    // Frame-time info
float glob_lastFrame = 0.0f;

float glob_animSpeed = 2.0f;                        // Animation attributes
bool glob_animate = false;

Camera camera = Camera();
float lastX, lastY, yaw, pitch;

bool pkeyPressed = false;                       // Helper variables for user interaction
bool spacebarPressed = false;
bool rmbClicked = false;
bool firstMouse = true;

// Forward declerations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursormove_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
int loadTexture(const char *texturePath);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    resolution[0] = width;
    resolution[1] = height;
    glob_aspectRatio = resolution[0] / resolution[1];
    camera.resetProjMat();
}

void cursormove_callback(GLFWwindow* window, double xpos, double ypos) {
    if (rmbClicked) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        xoffset *= camera.getSensitivity();
        yoffset *= camera.getSensitivity();

        camera.setYaw(camera.getYaw() + xoffset);
        camera.setPitch(camera.getPitch() + yoffset);

        glm::vec3 direction;
        direction.x = cos(glm::radians(camera.getYaw())) * cos(glm::radians(camera.getPitch()));
        direction.y = sin(glm::radians(camera.getPitch()));
        direction.z = sin(glm::radians(camera.getYaw())) * cos(glm::radians(camera.getPitch()));
        camera.setDir(glm::normalize(direction));
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        rmbClicked = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) { 
        rmbClicked = false;
        firstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.setZoom(camera.getZoom() + yoffset);
    camera.resetProjMat();
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        firstMouse = true;
        camera.reset();
    }

    glm::vec3 newSpeed = camera.getSpeed();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (newSpeed[2] < 0) { newSpeed[2] *= camera.getDecelFactor(); }
        newSpeed[2] = newSpeed[2] + camera.getAccelFactor();
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (newSpeed[2] > 0) { newSpeed[2] *= camera.getDecelFactor(); }
        newSpeed[2] = newSpeed[2] - camera.getAccelFactor();
    } else { newSpeed[2] *= camera.getDecelFactor(); }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (newSpeed[0] > 0) { newSpeed[0] *= camera.getDecelFactor(); }
        newSpeed[0] = newSpeed[0] - camera.getAccelFactor();
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (newSpeed[0] < 0) { newSpeed[0] *= camera.getDecelFactor(); }
        newSpeed[0] = newSpeed[0] + camera.getAccelFactor();
    } else { newSpeed[0] *= camera.getDecelFactor(); }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (newSpeed[1] < 0) { newSpeed[1] *= camera.getDecelFactor(); }
        newSpeed[1] = newSpeed[1] + camera.getAccelFactor();
    } else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (newSpeed[1] > 0) { newSpeed[1] *= camera.getDecelFactor(); }
        newSpeed[1] = newSpeed[1] - camera.getAccelFactor();
    } else { newSpeed[1] *= camera.getDecelFactor(); }
    camera.setSpeed(newSpeed);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacebarPressed) {
        spacebarPressed = true;
        glob_animate = !glob_animate;
    } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && spacebarPressed) {
        spacebarPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pkeyPressed) {
        pkeyPressed = true;
        camera.setPerspective(!camera.getPerspective());
        camera.resetProjMat();
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && pkeyPressed) {
        pkeyPressed = false;
    }
}

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
    GLFWwindow* window = glfwCreateWindow(resolution[0], resolution[1], "Modular Robotics", NULL, NULL);
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
    glViewport(0, 0, resolution[0], resolution[1]);
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

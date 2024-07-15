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

#define AUTO_ROTATE 0

std::unordered_map<int, Cube*> glob_objects;        // Hashmap of all <ID, object>. Global variable
glm::mat4 glob_viewmat, glob_projmat;           // View and Projection matrices

float resolution[2] = {1280.0f, 720.0f};        // Screen attributes
float asprat = resolution[0] / resolution[1];

const char *vertexShaderPath = "resources/shaders/vshader.glsl";    // Resource paths
const char *fragmentShaderPath = "resources/shaders/fshader.glsl";
const char *texturePath = "resources/textures/face_debug.png";

float glob_deltaTime = 0.0f;                    // Frame-time info
float glob_lastFrame = 0.0f;

float glob_animSpeed = 2.0f;                        // Animation attributes
bool glob_animate = false;

glm::vec3 cameraPos, cameraDirection, cameraUp, cameraSpeed; // Camera variables initialized in resetCamera()
float lastX, lastY, yaw, pitch;
const float CAMERA_MAX_SPEED = 25.0f;           // Camera attributes
const float CAMERA_ACCEL = 0.10f;
const float CAMERA_DECEL_FACTOR = 0.95f;
const float CAMERA_SENSITIVITY = 0.1f;
float FOV = 60.0f;
float cameraZoom = 0.0f;
bool perspective = true;

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
void resetCamera();
void resetProjMat();

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    resolution[0] = width;
    resolution[1] = height;
    asprat = resolution[0] / resolution[1];
    resetProjMat();
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

        xoffset *= CAMERA_SENSITIVITY;
        yoffset *= CAMERA_SENSITIVITY;

        yaw += xoffset;
        pitch += yoffset;

        yaw = std::fmod(yaw, 360.0f);
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection = glm::normalize(direction);
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
    cameraZoom += yoffset;
    cameraZoom = glm::clamp(cameraZoom, -25.0f, 25.0f);
    resetProjMat();
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        firstMouse = true;
        resetCamera();
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (cameraSpeed[2] < 0) { cameraSpeed[2] *= CAMERA_DECEL_FACTOR; }
        cameraSpeed[2] = cameraSpeed[2] + CAMERA_ACCEL;
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (cameraSpeed[2] > 0) { cameraSpeed[2] *= CAMERA_DECEL_FACTOR; }
        cameraSpeed[2] = cameraSpeed[2] - CAMERA_ACCEL;
    } else { cameraSpeed[2] *= CAMERA_DECEL_FACTOR; }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (cameraSpeed[0] > 0) { cameraSpeed[0] *= CAMERA_DECEL_FACTOR; }
        cameraSpeed[0] = cameraSpeed[0] - CAMERA_ACCEL;
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (cameraSpeed[0] < 0) { cameraSpeed[0] *= CAMERA_DECEL_FACTOR; }
        cameraSpeed[0] = cameraSpeed[0] + CAMERA_ACCEL;
    } else { cameraSpeed[0] *= CAMERA_DECEL_FACTOR; }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (cameraSpeed[1] < 0) { cameraSpeed[1] *= CAMERA_DECEL_FACTOR; }
        cameraSpeed[1] = cameraSpeed[1] + CAMERA_ACCEL;
    } else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (cameraSpeed[1] > 0) { cameraSpeed[1] *= CAMERA_DECEL_FACTOR; }
        cameraSpeed[1] = cameraSpeed[1] - CAMERA_ACCEL;
    } else { cameraSpeed[1] *= CAMERA_DECEL_FACTOR; }

    cameraSpeed = glm::step(0.00001f, glm::abs(cameraSpeed)) * cameraSpeed;
    cameraSpeed = glm::clamp(cameraSpeed, -CAMERA_MAX_SPEED, CAMERA_MAX_SPEED);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacebarPressed) {
        spacebarPressed = true;
        glob_animate = !glob_animate;
    } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && spacebarPressed) {
        spacebarPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pkeyPressed) {
        pkeyPressed = true;
        perspective = !perspective;
        resetProjMat();
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && pkeyPressed) {
        pkeyPressed = false;
    }
}

void resetProjMat() {
    float scalar;
    if (perspective) {
        scalar = 2.0 / (1.0 + std::exp(cameraZoom / 4.0));
        glob_projmat = glm::perspective(glm::radians(FOV * scalar), asprat, 0.1f, 100.0f); 
    } else {
        scalar = 2.0 / (1.0 + std::exp(cameraZoom / 10.0));
        scalar *= scalar;
        glob_projmat = glm::ortho(-2.0f * asprat * scalar, 2.0f * asprat * scalar, -2.0f * scalar, 2.0f * scalar, 0.1f, 100.0f); 
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

void resetCamera() {
    cameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
    cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp = glm::cross(cameraDirection, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDirection)));
    cameraSpeed = glm::vec3(0.0f, 0.0f, 0.0f);
    lastX = resolution[0]/2.0;
    lastY = resolution[1]/2.0;
    yaw = 270.0f;
    pitch = 0.0f;
    cameraZoom = 0.0f;
    resetProjMat();
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
    
    resetCamera();
    glob_viewmat = glm::mat4(1.0f);
    glob_projmat = glm::perspective(glm::radians(45.0f), asprat, 0.1f, 100.0f);

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
        cameraPos += (cameraSpeed.z * cameraDirection * glob_deltaTime);
        cameraPos += (cameraSpeed.x * glm::cross(cameraDirection, cameraUp) * glob_deltaTime);
        cameraPos += (cameraSpeed.y * cameraUp * glob_deltaTime);
        glob_viewmat = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
#else
        cameraPos = glm::rotate(cameraPos, glm::radians(15.0f * glob_deltaTime), glm::vec3(0.0, 1.0, 0.0));
        glob_viewmat = glm::lookAt(cameraPos, glm::vec3(0.0, 0.0, 0.0), cameraUp);
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

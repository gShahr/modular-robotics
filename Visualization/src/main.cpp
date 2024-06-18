#include <iostream>
#include <stdio.h>
#include <math.h>
#include <algorithm> // clamp

#include "glad/glad.h"
#include "glfw3.h"
#include "stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Cube.hpp"
#include "ObjectCollection.hpp"

float resolution[2] = {800.0f, 600.0f};
float asprat = resolution[0] / resolution[1];

const char *vertexShaderPath = "resources/shaders/vshader.glsl";
const char *fragmentShaderPath = "resources/shaders/fshader.glsl";

float deltaTime = 0.0f;
float lastFrame = 0.0f;

const float CAMERA_MAX_SPEED = 25.0f;
const float CAMERA_ACCEL = 0.10f;
const float CAMERA_DECEL_FACTOR = 0.95f;
const float CAMERA_SENSITIVITY = 0.1f;
float FOV = 45.0f;
float cameraZoom = 0.0f;
bool perspective = true;
glm::vec3 cameraPos; // Camera variables initialized in resetCamera()
glm::vec3 cameraDirection;
glm::vec3 cameraUp;
glm::vec3 cameraSpeed;
float lastX;
float lastY;
float yaw;
float pitch;
bool pkeyPressed = false;
bool rmbClicked = false;
bool firstMouse = true;

glm::mat4 viewmat, projmat, transform;

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

        pitch = std::clamp(pitch, -89.0f, 89.0f);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection = glm::normalize(direction);
        //std::cout << "POS: " << glm::to_string(cameraPos) << " PITCH/YAW: " << pitch << " | " << yaw << " | DIR: " << glm::to_string(cameraDirection) << std::endl;
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
    cameraZoom = std::clamp(cameraZoom, -25.0f, 25.0f);
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

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pkeyPressed) {
        pkeyPressed = true;
        perspective = !perspective; // TODO Scroll controls FOV for perspective, frustrum size for ortho
        resetProjMat();
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && pkeyPressed) {
        pkeyPressed = false;
    }
}

void resetProjMat() {
    float scalar;
    if (perspective) {
        scalar = 2.0 / (1.0 + std::exp(cameraZoom / 4.0));
        projmat = glm::perspective(glm::radians(FOV * scalar), asprat, 0.1f, 100.0f); 
    } else {
        scalar = 2.0 / (1.0 + std::exp(cameraZoom / 10.0));
        scalar *= scalar;
        projmat = glm::ortho(-2.0f * asprat * scalar, 2.0f * asprat * scalar, -2.0f * scalar, 2.0f * scalar, 0.1f, 100.0f); 
    }
    // std::cout << "cameraZoom: " << cameraZoom << " | scalar: " << scalar << std::endl;
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
    cameraPos = glm::vec3(0.0f, 0.0f, -6.0f);
    cameraDirection = glm::vec3(0.0f, 0.0f, 1.0f);
    cameraUp = glm::cross(cameraDirection, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDirection)));
    cameraSpeed = glm::vec3(0.0f, 0.0f, 0.0f);
    lastX = resolution[0]/2.0;
    lastY = resolution[1]/2.0;
    yaw = 90.0f;
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

    Shader shader = Shader(vertexShaderPath, fragmentShaderPath);
    int texture = loadTexture("resources/textures/face_debug.png");
    unsigned int VAO = _createCubeVAO();

    ObjectCollection* cubes = new ObjectCollection(&shader, VAO, texture);
    cubes->addObj(new Cube(1.0f, 0.0f, 0.0f));  // Bottom layer
    cubes->addObj(new Cube(0.0f, 0.0f, 0.0f));
    cubes->addObj(new Cube(0.0f, 0.0f, -1.0f));
    cubes->addObj(new Cube(0.0f, 1.0f, -1.0f));  // Middle layer part 1
    cubes->addObj(new Cube(0.0f, 1.0f, -2.0f));
    cubes->addObj(new Cube(1.0f, 1.0f, -2.0f));
    cubes->addObj(new Cube(1.0f, 2.0f, -2.0f)); // Top layer
    cubes->addObj(new Cube(2.0f, 2.0f, -2.0f));
    cubes->addObj(new Cube(2.0f, 2.0f, -1.0f));
    cubes->addObj(new Cube(2.0f, 1.0f, -1.0f));  // Middle layer part 2
    cubes->addObj(new Cube(2.0f, 1.0f, 0.0f));
    cubes->addObj(new Cube(1.0f, 1.0f, 0.0f));

    resetCamera();
    viewmat = glm::mat4(1.0f);
    transform = glm::mat4(1.0f);
    projmat = glm::perspective(glm::radians(45.0f), asprat, 0.1f, 100.0f);
    transform = glm::scale(transform, glm::vec3(0.9f, 0.9f, 0.9f));


    while(!glfwWindowShouldClose(window)) {
        // -- Input ---
        
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        cameraPos += (cameraSpeed.z * cameraDirection * deltaTime);
        cameraPos += (cameraSpeed.x * glm::cross(cameraDirection, cameraUp) * deltaTime);
        cameraPos += (cameraSpeed.y * cameraUp * deltaTime);
        viewmat = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);

        // -- Rendering --
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // std::cout << glm::to_string(viewmat) << std::endl;
        cubes->drawAll();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
    std::cout << "Goodbye, world\n";
    return 0;
}

#include "glfw3.h"
#include "Camera.hpp"

extern Camera camera;
extern bool glob_animate;
extern float glob_resolution[2];
extern float glob_aspectRatio;

float lastX, lastY, yaw, pitch;         // Helper variables for user interaction
bool pkeyPressed = false;
bool spacebarPressed = false;
bool rmbClicked = false;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glob_resolution[0] = width;
    glob_resolution[1] = height;
    glob_aspectRatio = glob_resolution[0] / glob_resolution[1];
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

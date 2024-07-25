#include "glfw3.h"
#include "Camera.hpp"

extern Camera camera;
extern float glob_resolution[2];
extern float glob_aspectRatio;
extern float glob_animSpeed, glob_deltaTime;
extern bool glob_animateAuto, glob_animateForward, glob_animateRequest;

float lastX, lastY, yaw, pitch;         // Helper variables for user interaction
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

glm::vec4 convertClickCoordToWorldDir(float xp, float yp) {
    // Objective: calculate the direction we clicked in, by
    //  transforming our click-point into world space, subtracting
    //  that point from our camera's position in world space,
    //  and normalizing.
    glm::mat4 invViewMat, invProjMat;
    glm::vec4 clickPoint, rayDir;

    invViewMat = glm::inverse(camera.getViewMat());
    invProjMat = glm::inverse(camera.getProjMat());

    clickPoint = glm::vec4( // Convert our click-coordinates into clip space (NDC; -1.0 to 1.0).
        (float)(xp / glob_resolution[0] * 2.0 - 1.0),
        (float)-(yp / glob_resolution[1] * 2.0 - 1.0), // (negative)
        0.0f, 1.0f);
    clickPoint = invProjMat * clickPoint;       // Convert to view space
    clickPoint = invViewMat * clickPoint;       // Convert to world space
    clickPoint = clickPoint / clickPoint[3];    // Normalize homogenous coord to 1

    rayDir = glm::normalize(clickPoint - glm::vec4(camera.getPos(), 1.0f));

    std::cout << "Click direction: " << glm::to_string(rayDir) << std::endl;

    return rayDir;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        rmbClicked = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) { 
        rmbClicked = false;
        firstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) { // Left-click
        double xp, yp;
        glfwGetCursorPos(window, &xp, &yp);
        convertClickCoordToWorldDir(xp, yp);
    }
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.setZoom(camera.getZoom() + yoffset);
    camera.resetProjMat();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {        // Escape -- close
        glfwSetWindowShouldClose(window, true);
    } else if ((key == GLFW_KEY_R) && (action == GLFW_PRESS)) {     // R -- reset camera
        firstMouse = true;
        camera.reset();
    } else if ((key == GLFW_KEY_SPACE) && (action == GLFW_PRESS)) { // Spacebar -- toggle auto-animation
        glob_animateAuto = !glob_animateAuto;
        glob_animateRequest = glob_animateAuto;
        std::cout << "Auto Animation " << (glob_animateAuto ? "ENABLED" : "DISABLED") << std::endl;
    } else if ((key == GLFW_KEY_P) && (action == GLFW_PRESS)) {      // P -- toggle perspective
        camera.setPerspective(!camera.getPerspective());
        camera.resetProjMat();
    } else if ((key == GLFW_KEY_RIGHT) && (action == GLFW_PRESS)) { // Right arrow -- 
        glob_animateForward = true;
        glob_animateRequest = true;
    } else if ((key == GLFW_KEY_LEFT) && (action == GLFW_PRESS)) {  // Left arrow --
        glob_animateForward = false;
        glob_animateRequest = true;
    } else if ((key == GLFW_KEY_UP) && (action != GLFW_RELEASE)) {
        glob_animSpeed = std::min(25.0f, glob_animSpeed * (action == GLFW_PRESS ? 1.2f : 1.05f));
        std::cout << "Animation speed RAISED to " << glob_animSpeed << std::endl;
    } else if ((key == GLFW_KEY_DOWN) && (action != GLFW_RELEASE)) {
        glob_animSpeed = std::max(0.5f, glob_animSpeed * (action == GLFW_PRESS ? 0.8333f : 0.96f));
        std::cout << "Animation speed LOWERED to " << glob_animSpeed << std::endl;
    }
}

void processInput(GLFWwindow *window) {

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

}

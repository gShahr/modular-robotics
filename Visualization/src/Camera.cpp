#include "Camera.hpp"

Camera::Camera() {
    this->reset();
}

void Camera::setPos(glm::vec3 pos) { this->pos = pos;}
void Camera::setFov(float fov) { this->fov = fov; }
void Camera::setSpeed(glm::vec3 speed) { this->speed = glm::clamp( glm::step(0.01f, glm::abs(speed)) * speed, -this->getMaxSpeed(), this->getMaxSpeed()); }
void Camera::setPerspective(bool p) { this->perspective = p; }
void Camera::setYaw(float yaw) { this->yaw = std::fmod(yaw, 360.0f); }
void Camera::setPitch(float pitch) { this->pitch = glm::clamp(pitch, -89.0f, 89.0f); }
void Camera::setZoom(float zoom) { this->zoom = glm::clamp(zoom, -25.0f, 25.0f); }
void Camera::setDir(glm::vec3 dir) { 
    this->dir = dir; 
    this->up = glm::cross(dir, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), dir)));
}

glm::vec3 Camera::getPos() { return this->pos; }
glm::vec3 Camera::getDir() { return this->dir; }
glm::vec3 Camera::getSpeed() { return this->speed; }
glm::vec3 Camera::getUp() { return this->up; }
glm::mat4 Camera::getViewMat() { return this->viewmat; }
glm::mat4 Camera::getProjMat() { return this->projmat; }
bool Camera::getPerspective() { return this->perspective; }
float Camera::getYaw() { return this->yaw; }
float Camera::getPitch() { return this->pitch; }
float Camera::getZoom() { return this->zoom; }
float Camera::getFov() { return this->fov; }

float Camera::getMaxSpeed() { return this->c_maxSpeed; }
float Camera::getAccelFactor() { return this->c_accelFactor; }
float Camera::getDecelFactor() { return this->c_decelFactor; }
float Camera::getSensitivity() { return this->c_sensitivity; }

void Camera::calcViewMat() {
    this->viewmat = glm::lookAt(this->getPos(), this->getPos() + this->getDir(), this->getUp());
}
void Camera::calcViewMat(glm::vec3 targetDir) {
    this->viewmat = glm::lookAt(this->getPos(), targetDir, this->getUp());
}

void Camera::resetProjMat() {
    float scalar;
    if (this->getPerspective()) {
        scalar = 2.0 / (1.0 + std::exp(this->getZoom() / 4.0));
        this->projmat = glm::perspective(glm::radians(this->getFov() * scalar), glob_aspectRatio, 0.1f, 100.0f); 
    } else {
        scalar = 2.0 / (1.0 + std::exp(this->getZoom() / 10.0));
        scalar *= scalar;
        this->projmat = glm::ortho(-2.0f * glob_aspectRatio * scalar, 2.0f * glob_aspectRatio * scalar, -2.0f * scalar, 2.0f * scalar, 0.1f, 100.0f); 
    }
}

void Camera::reset() {
    glm::vec3 _defaultPos = glm::vec3(0.0f, 0.0f, 6.0f);
    glm::vec3 _defaultDir = glm::vec3(0.0f, 0.0f, -1.0f);

    this->setPos(_defaultPos);
    this->setFov(60.0f);
    this->setSpeed(glm::vec3(0.0f));
    this->setPerspective(true);
    this->setYaw(270.0f);
    this->setPitch(0.0f);
    this->setZoom(0.0f);
    this->setDir(_defaultDir);

    this->resetProjMat();
}

#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

extern float glob_resolution[2];
extern float glob_aspectRatio;

class Camera
{
public:
    Camera();
    void calcViewMat();
    void calcViewMat(glm::vec3 targetDir);
    void reset();
    void resetProjMat();

    void setPos(glm::vec3 pos);
    void setFov(float fov);
    void setSpeed(glm::vec3 speed);
    void setPerspective(bool p);
    void setYaw(float yaw);
    void setPitch(float pitch);
    void setZoom(float zoom);
    void setDir(glm::vec3 dir);

    glm::vec3 getPos();
    glm::vec3 getDir();
    glm::vec3 getSpeed();
    glm::vec3 getUp();
    glm::mat4 getViewMat();
    glm::mat4 getProjMat();
    bool getPerspective();
    float getYaw();
    float getPitch();
    float getZoom();
    float getFov();

    float getMaxSpeed();
    float getAccelFactor();
    float getDecelFactor();
    float getSensitivity();

private:
    // Accessible with getters/setters
    float fov, zoom;
    float yaw, pitch;
    bool perspective;
    glm::vec3 pos, dir, up, speed;
    glm::mat4 viewmat, projmat;

    // Internal
    float _lastX, _lastY;

    // Hardcoded values
    const float c_maxSpeed = 25.0f;
    const float c_accelFactor = 0.10f;
    const float c_decelFactor = 0.95f;
    const float c_sensitivity = 0.1f;
};

#endif

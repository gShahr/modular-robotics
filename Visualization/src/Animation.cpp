#include "Animation.hpp"

Animation::Animation(glm::vec3 AnchorDirection, glm::vec3 DeltaPos) {
    this->AnchorDirection = AnchorDirection;
    this->DeltaPos = DeltaPos;
    this->PreTranslation = glm::clamp((0.5f * AnchorDirection) + (0.5f * DeltaPos), -0.5f, 0.5f) + 0.0f;
    this->MaxAngle = (abs(DeltaPos[0]) + abs(DeltaPos[1]) + abs(DeltaPos[2])) * 90.0f;
    this->RotationAxis = glm::normalize(glm::cross(DeltaPos, AnchorDirection)) + 0.0f;

    if (std::isnan(this->RotationAxis[0])) {
        throw std::domain_error("Failed to calculate rotation axis for animation: target path is parallel to anchor direction");
    }

    std::cout << "AnchorDirection: " << glm::to_string(this->AnchorDirection) << std::endl;
    std::cout << "DeltaPos: " << glm::to_string(this->DeltaPos) << std::endl;
    std::cout << "PreTranslation: " << glm::to_string(this->PreTranslation) << std::endl;
    std::cout << "MaxAngle: " << this->MaxAngle << std::endl;
    std::cout << "RotationAxis: " << glm::to_string(this->RotationAxis) << std::endl;
}


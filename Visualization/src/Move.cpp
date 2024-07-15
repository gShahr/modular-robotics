#include "Move.hpp"

Move::Move(int moverId, glm::vec3 anchorDir, glm::vec3 deltaPos, bool sliding) {
    this->moverId = moverId;
    this->anchorDir = anchorDir;
    this->deltaPos = deltaPos;
    this->sliding = sliding;

    this->maxAngle = 0;
    this->preTrans = glm::vec3(0.0f);
    this->rotAxis = glm::vec3(0.0f);

    if (!sliding) {
        this->preTrans = glm::clamp((0.5f * anchorDir) + (0.5f * deltaPos), -0.5f, 0.5f) + 0.0f;
        this->maxAngle = (abs(deltaPos[0]) + abs(deltaPos[1]) + abs(deltaPos[2])) * 90.0f;
        this->rotAxis = glm::normalize(glm::cross(deltaPos, anchorDir)) + 0.0f;
    }
}

Move* Move::copy() {
    return new Move(this->moverId, this->anchorDir, this->deltaPos, this->sliding);
}

Move* Move::reverse() {
    glm::vec3 deltaPos = -this->deltaPos;
    glm::vec3 anchorDir = this->anchorDir;

    if (abs(anchorDir[0]) + abs(anchorDir[1]) + abs(anchorDir[2]) > 0.1f) { // If anchorDir is uniform 0.0f, indicates a generic sliding move; bypass the anchorDir changeups
        // If it's a diagonal sliding move or a corner pivot move, we need to do some math
        if (glm::dot(glm::abs(deltaPos), glm::vec3(1.0f)) > 1.0f) {
            anchorDir = (glm::vec3(1.0f) - glm::abs(anchorDir)) * deltaPos;
            if (this->sliding) { anchorDir = glm::abs(anchorDir); }
        }
    }

    return new Move(moverId, anchorDir, deltaPos, this->sliding);
}

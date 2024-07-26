#include "Move.hpp"

void _construct(Move* move, int moverId, glm::vec3 anchorDir, glm::vec3 deltaPos, bool sliding, bool checkpointMove) {
    move->moverId = moverId;
    move->anchorDir = anchorDir;
    move->deltaPos = deltaPos;
    move->sliding = sliding;
    move->checkpointMove = checkpointMove;

    move->maxAngle = 0;
    move->preTrans = glm::vec3(0.0f);
    move->rotAxis = glm::vec3(0.0f);

    if (!sliding) {
        move->preTrans = glm::clamp((0.5f * anchorDir) + (0.5f * deltaPos), -0.5f, 0.5f) + 0.0f;
        move->maxAngle = (abs(deltaPos[0]) + abs(deltaPos[1]) + abs(deltaPos[2])) * 90.0f;
        move->rotAxis = glm::normalize(glm::cross(deltaPos, anchorDir)) + 0.0f;
    }
}

Move::Move(int moverId, glm::vec3 anchorDir, glm::vec3 deltaPos, bool sliding) {
    _construct(this, moverId, anchorDir, deltaPos, sliding, false);
}
Move::Move(int moverId, glm::vec3 anchorDir, glm::vec3 deltaPos, bool sliding, bool checkpoint) {
    _construct(this, moverId, anchorDir, deltaPos, sliding, checkpoint);
}

Move* Move::copy() {
    return new Move(this->moverId, this->anchorDir, this->deltaPos, this->sliding, this->checkpointMove);
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

    return new Move(moverId, anchorDir, deltaPos, this->sliding, this->checkpointMove);
}

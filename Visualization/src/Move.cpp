#include "Move.hpp"

Move::Move(int moverId, int anchorId, glm::vec3 deltaPos) {
    this->moverId = moverId;
    this->anchorId = anchorId;
    this->deltaPos = deltaPos;
}

#ifndef MOVE_H
#define MOVE_H

#include "glm/glm.hpp"

class Move
{
public:
    Move(int moverId, int anchorId, glm::vec3(deltaPos));

    int anchorId, moverId;
    glm::vec3 deltaPos;
};

#endif

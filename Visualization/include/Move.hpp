#ifndef MOVE_H
#define MOVE_H

#include <iostream>
#include "glm/glm.hpp"

class Move
{
public:
    Move(int moverId, glm::vec3 anchorDir, glm::vec3 deltaPos, bool sliding);
    Move(int moverId, glm::vec3 anchorDir, glm::vec3 deltaPos, bool sliding, bool checkpointMove);
    Move* copy();           // Creates a deep copy of a Move
    Move* reverse();        // Creates a new move which is the "reverse" of this Move

    int moverId;            // Which module is moving
    glm::vec3 anchorDir;    // Vector which determines the "anchor" face of a move (or, for diagonal sliding moves, which direction to move FIRST)
    glm::vec3 deltaPos;     // Final position offset
    bool sliding;           // Whether or not this is a sliding move
    bool checkpointMove;    // Whether or not this is a checkpoint move

    float maxAngle;         // Total angle offset (for pivot moves)
    glm::vec3 preTrans;     // Required amount of pre-translation (for pivot moves)
    glm::vec3 rotAxis;      // Axis of rotation (for pivot moves)
};

#endif

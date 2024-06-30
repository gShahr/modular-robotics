#ifndef ANIMATIONSEQUENCE_H
#define ANIMATIONSEQUENCE_H

#include <deque>
#include <vector>
#include <stack>
#include "Animation.hpp"

class AnimationSequence
{
public:
    AnimationSequence(std::vector<Animation*> anims);
    Animation* pop();
    Animation* undo();

    std::deque<Animation*> anims;
    std::stack<Animation*> undostack;
    int totalMoves, remainingMoves, currentMove;

};

#endif

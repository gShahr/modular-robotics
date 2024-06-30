#include "AnimationSequence.hpp"

AnimationSequence::AnimationSequence(std::vector<Animation*> anims) {
    this->anims = {};
    this->undostack = {};
    for (Animation* anim : anims) {
        this->anims.push_back(anim);
    }
    this->totalMoves = anims.size();
    this->remainingMoves = anims.size();
    this->currentMove = 0;
    return;
}

Animation* AnimationSequence::pop() {
    if (this->remainingMoves == 0) { return NULL; }

    Animation* anim = this->anims.front();
    this->anims.pop_front();
    this->undostack.push(anim);

    this->remainingMoves--;
    this->currentMove++;
    
    return anim;
}

Animation* AnimationSequence::undo() {
    if (this->currentMove == 0) { return NULL; }

    Animation* anim = this->undostack.top();
    this->anims.push_front(anim);
    this->undostack.pop();

    this->remainingMoves++;
    this->currentMove--;

    return anim;
}

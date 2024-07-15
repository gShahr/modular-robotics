#ifndef MOVESEQUENCE_H
#define MOVESEQUENCE_H

#include <deque>
#include <vector>
#include <stack>
#include "Move.hpp"

class MoveSequence
{
public:
    MoveSequence(std::vector<Move*> moves);
    Move* pop();
    Move* undo();

    std::deque<Move*> moves;
    std::stack<Move*> undostack;
    int totalMoves, remainingMoves, currentMove;

};

#endif

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
    Move* pop();            // Deep-copy the next move in the queue, and pop it
    Move* undo();           // Deep-copy the top move in the undostack, and pop it
    Move* peek();           // Peek at the next move in the queue
    Move* peekBack();       // Peek at the previous move (top of the undo stack)

    std::deque<Move*> moves;
    std::stack<Move*> undostack;
    int totalMoves, remainingMoves, currentMove;

};

#endif

#include "MoveSequence.hpp"

MoveSequence::MoveSequence(std::vector<Move*> moves) {
    this->moves = std::deque<Move*>();
    this->undostack = std::stack<Move*>();
    for (Move* move : moves) {
        this->moves.push_back(move);
    }
    this->totalMoves = moves.size();
    this->remainingMoves = moves.size();
    this->currentMove = 0;
    return;
}

Move* MoveSequence::pop() {
    if (this->remainingMoves == 0) { return NULL; }

    Move* move = this->moves.front();
    this->moves.pop_front();
    this->undostack.push(move);

    move = move->copy();

    this->remainingMoves--;
    this->currentMove++;

    return move;
}

Move* MoveSequence::undo() {
    if (this->currentMove == 0) { return NULL; }

    Move* move = this->undostack.top();
    this->moves.push_front(move);
    this->undostack.pop();

    move = move->reverse();

    this->remainingMoves++;
    this->currentMove--;

    return move;
}

Move* MoveSequence::peek() {
    if (this->remainingMoves == 0) { return NULL; }
    return this->moves.front();
}

Move* MoveSequence::peekBack() {
    if (this->currentMove == 0) { return NULL; }
    return this->undostack.top();
}

export class MoveSequence {
    constructor(moves = []) {
        this.moves = moves;
        this.undostack = [];
        this.totalMoves = moves.length;
        this.remainingMoves = moves.length;
        this.currentMove = 0;
    }

    pop() {
        if (this.remainingMoves == 0) {
            return;
        }

        let move = this.moves.shift();

        this.remainingMoves--;
        this.currentMove++;

        this.undostack.push(move);
        
        return move;
    }

    undo() {
        if (this.remainingMoves == 0) {
            return;
        }

        let move = this.undostack.pop();

        this.remainingMoves++;
        this.currentMove--;

        this.moves.unshift(move);

        return move;
    }
}

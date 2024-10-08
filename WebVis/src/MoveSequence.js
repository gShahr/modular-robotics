export class MoveSequence {
    constructor(moves = []) {
        this.moves = moves;
        this.undostack = [];
        this.totalMoves = moves.length;
        this.remainingMoves = moves.length;
        this.currentMove = 0;
        this.updateMoveProgressString();
    }

    updateMoveProgressString() {
        this.moveProgressString = `Move #${this.currentMove} / #${this.totalMoves}`;
        document.getElementById("infoOverlay").innerHTML = this.moveProgressString;
    }

    pop() {
        if (this.remainingMoves == 0) {
            return;
        }

        let move = this.moves.shift();

        this.remainingMoves--;
        this.currentMove++;

        this.undostack.push(move);
        
        this.updateMoveProgressString();
        return move;
    }

    undo() {
        if (this.currentMove == 0) {
            return;
        }

        let move = this.undostack.pop();

        this.remainingMoves++;
        this.currentMove--;

        this.moves.unshift(move);

        this.updateMoveProgressString();
        return move.reverse();
    }
}

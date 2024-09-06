import * as THREE from 'three';
import { MoveType } from './utils.js';

export class Move{
    constructor(id, anchorDir, deltaPos, moveType, checkpoint) { 
        this.id = id;
        this.anchorDir = anchorDir;
        this.deltaPos = deltaPos;
        this.moveType = moveType;
        this.checkpoint = checkpoint;

        this.maxAngle = 0;
        this.preTrans = new THREE.Vector3(1.0, 0.0, 0.0);
        this.rotAxis = new THREE.Vector3(1.0, 0.0, 0.0);

        switch(moveType) {
            case MoveType.PIVOT: {
                let _minPreTrans = new THREE.Vector3(-0.5, -0.5, -0.5);
                let _maxPreTrans = new THREE.Vector3(0.5, 0.5, 0.5);
                this.preTrans = anchorDir.clone().add(deltaPos).multiplyScalar(0.5).clamp(_minPreTrans, _maxPreTrans);
                this.postTrans = this.preTrans.clone().negate();

                this.rotAxis = deltaPos.clone().cross(anchorDir).normalize();

                this.maxAngle = THREE.MathUtils.degToRad(deltaPos.toArray().reduce((p,a)=>p+Math.abs(a), 0.0) * 90.0);
                break;
                } 
            case MoveType.SLIDING: { // No additional attributes needed
                break;
            }
            case MoveType.MONKEY: {
                break;
            }
        }
    }

    reverse() {
        let newMove;
        let newDeltaPos = this.deltaPos.clone().negate();
        let newAnchorDir = this.anchorDir.clone();
        let testVec = new THREE.Vector3(1.0, 1.0, 1.0);
        let manhattanDistance = newDeltaPos.abs().clone().dot(testVec);

        // Test for manhattanDistance < 3.0. Any valid move should have
        // distance of exactly 1 or 2; if greater, either there's a bug
        // with the scenario, or it's a deliberately-exaggerated slide 
        // move for which the anchorDir should be 0 anyway.
        if (manhattanDistance < 3.0) {
            // If corner move, calculate new anchor dir
            if (manhattanDistance > 1.0) { 
                newAnchorDir = testVec.sub(this.anchorDir.abs()).multiply(newDeltaPos);
                
                // Sliding moves use only positive numbers for encoding anchor directions
                if (this.moveType == MoveType.SLIDING && manhattanDistance < 3.0) { 
                    newAnchorDir = newAnchorDir.abs(); 
                }
            }
        }
        return new Move(this.id, newAnchorDir, newDeltaPos, this.moveType, this.checkpoint);
    }
}

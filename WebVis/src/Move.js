import * as THREE from 'three';
import { MoveType } from './utils.js';

export class Move{
    constructor(id, anchorDir, deltaPos, moveType) { 
        this.id = id;
        this.anchorDir = anchorDir;
        this.deltaPos = deltaPos;
        this.moveType = moveType;

        this.maxAngle = 0;
        this.preTrans = 0;
        this.rotAxis = 0;

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
            case MoveType.SLIDING: {
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
        if (newDeltaPos.abs().clone().dot(testVec) > 1.0) { // If corner move, calculate new anchor dir
            newAnchorDir = testVec.sub(this.anchorDir.abs()).multiply(newDeltaPos);
        }
        return new Move(this.id, newAnchorDir, newDeltaPos, this.moveType);
    }
}

import * as THREE from 'three';
import { ModuleType, MoveType } from './utils.js';

export class Move{
    constructor(id, anchorDir, deltaPos, moveType, checkpoint, moduleType = ModuleType.CUBE) { 
        this.id = id;
        this.anchorDir = anchorDir;
        this.deltaPos = deltaPos;
        this.moveType = moveType;
        this.checkpoint = checkpoint;

        this.moduleType = moduleType;
        switch (moduleType) {
            case ModuleType.CUBE: this.dihedralAngle = 90.0; this.inscsphere = 0.5; this.midsphere = 0.7071; break;
            case ModuleType.RHOMBIC_DODECAHEDRON: this.dihedralAngle = 60.0; this.inscsphere = 0.7071; this.midsphere = 0.8165; break;
        }

        this.maxAngle = 0;
        this.preTrans = new THREE.Vector3(1.0, 0.0, 0.0);
        this.rotAxis = new THREE.Vector3(1.0, 0.0, 0.0);

        // PIVOTING RHOMBIC DODECAHEDRONS:
        //  Pivoting a shape is a translate->rotate->untranslate operation.
        //  The AXIS OF ROTATION is easy --
        //      Simply take the cross-product of the face-normal and the delta-position.
        //  To make this rotation happen about a specific point,
        //      we need to translate the shape such that the point lies at the origin.
        //      (For rotation about an EDGE, we select the midpoint of the edge.)
        //  The translation DIRECTION is trickier.
        //  We need to point a vector from the origin of the shape to the corresponding edge.
        //  For that, we need to figure out which edge we're pivoting about.
        //  However, all we have is the face-normal ("anchor direction"), and the delta-position.
        //
        //  The coordinate system used has its "absolute-origin" at the center of the shape that we are pivoting around.
        //  That is, the origin does NOT lie in the pivoting shape; it's in the "anchor" shape!
        //
        //  During the pivot, the origin of the shape traverses from a start position to an end position.
        //  Consider if this traversal was a linear slide from the start to the end:
        //      then, there is an "average position" of this linear translation,
        //      and we can draw a vector from the absolute-origin to this "average position".
        //  This "average position" lies at the midpoint of the edge about which we're pivoting.

        switch(moveType) {
            case MoveType.PIVOT: {
                // Rotation axis
                this.rotAxis = this.deltaPos.clone().cross(this.anchorDir).normalize();

                // Determine our start- and end-positions,
                //  in the coordinate system centered at the origin of the "anchor" shape
                //  (This happens to be neatly encoded in anchorDir;
                //   we just need to re-scale it to appropriate length)
                let _startPos = this.anchorDir.clone().multiplyScalar(this.inscsphere * 2);

                // Determine the midpoint of the linear-translation
                let _linearTranslation = _startPos.clone().add(this.deltaPos);

                let _translationDir = _linearTranslation.clone().normalize();

                console.log({deltaPos, _startPos, _linearTranslation, _translationDir});

                // Scale to midsphere length
                this.postTrans = _translationDir.clone().multiplyScalar(this.midsphere);
                this.preTrans = this.postTrans.clone().negate();

                this.maxAngle = THREE.MathUtils.degToRad(this.deltaPos.abs().sum() * this.dihedralAngle);
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
        let newDeltaPos = this.deltaPos.clone().negate();

        // In the coordinate system centered at the origin of the "anchor" shape,
        //  take our position and subtract the delta-position of the move.
        //  This results in the end-position of the move, which...
        //  happens to correspond neatly the anchor direction in this coordinate system.
        //      (Same property that allows us to identify our position in this coordinate system)
        let newAnchorDir = this.anchorDir.clone().multiplyScalar(this.inscsphere * 2).sub(this.deltaPos).normalize();

        return new Move(this.id, newAnchorDir, newDeltaPos, this.moveType, this.checkpoint, this.moduleType);
    }
}

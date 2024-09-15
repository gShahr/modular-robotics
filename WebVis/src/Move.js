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
        let _d_cf, _d_fe;
        // d_cf = distance from 3d center to 2d/face center
        // d_fe = distance from 2d/face center to 1d/edge center 
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
        //  For rotation about an EDGE, we select the midpoint of the edge.
        //  A shape's midsphere is a sphere which is tangent to every edge of the shape.
        //      For a regular polyhedron, that tangent point happens to lie on the midpoint of each edge.
        //      This makes the LENGTH of our translation trivial -- it's just the radius of the midsphere.
        //  The translation DIRECTION is much trickier.
        //  We need to point a vector from the origin of the shape to the corresponding edge.
        //  For that, we need to figure out which edge we're pivoting about.
        //  However, all we have is the face-normal ("anchor direction"), and the delta-position.
        //  Somewhere, we need to reconstruct the translation direction from that information.
        //
        //  (I derived this by dumb luck, by staring at cases until patterns emerged; 
        //   I don't know what properties allow the following to be true. So it's magic, for now.)
        //  
        //  The coordinate system used has its "absolute-origin" at the center of the shape that we are pivoting around.
        //  That is, the origin does NOT lie in the pivoting shape; it's in the "anchor" shape!
        //
        //  During the pivot, the origin of the shape traverses from a start position to an end position.
        //  Consider if this traversal was a linear slide from the start to the end:
        //      then, there is an "average position" of this linear translation,
        //      and we can draw a vector from the absolute-origin to this "average position".
        //      (We don't normalize this vector -- not yet!)
        //
        //  But we're pivoting: the origin of the shape actually follows an arc, not a line segment.
        //      We need to correct for this arc. It so happens that, for RD's, all valid moves
        //      have delta positions which result in a movement of +/- 1 on exactly 2 axis.
        //      (E.g. <0, 0, 1>, <0, -1, 0>, etc. are not valid delta positions;
        //       but <-1, 0, 1>, <1, 1, 0>, <0, -1, -1> etc. are)
        //
        //  One axis is always "untouched". This axis is where we need to add the correction factor.
        //      The magnitude of the correction factor is 0.5: we need to SHRINK the length of the
        //      corresponding element in the linear-translation vector by 0.5.
        //
        //  Applying this correction factor yields a vector in the direction of the midpoint of the correct edge.
        //  So we can simply scale it to the correct length (the midsphere radius) and use it as our translation vector.

        switch(moveType) {
            case MoveType.PIVOT: {
                // Rotation axis
                this.rotAxis = deltaPos.clone().cross(anchorDir).normalize();

                // Determine our start- and end-positions,
                //  in the coordinate system centered at the origin of the "anchor" shape
                //  (This happens to be neatly encoded in anchorDir;
                //   we just need to re-scale it to appropriate length)
                let _startPos = anchorDir.clone().multiplyScalar(this.inscsphere * 2);
                let _endPos = _startPos.clone().add(deltaPos);

                // Determine the midpoint of the linear-translation
                let _linearTranslation = _startPos.clone().multiplyScalar(2).add(deltaPos);

                // Calculate our initial pass at the arc-correction factor:
                //  Extract the 0-element of the delta-position
                let _arcTranslation = new THREE.Vector3(1.0, 1.0, 1.0)
                    .sub(deltaPos.clone().abs());

                // Need to determine what sign this arc-correction factor should have
                //  (opposite the corresponding element in the linear-translation vector)
                let _arcSign = Math.sign(
                    new THREE.Vector3()
                    .multiplyVectors(_arcTranslation, _linearTranslation) // element-wise multiplication
                    .negate().sum());

                // Apply our arc correction factor to the linear translation and normalize
                let _translationDir = _linearTranslation.clone().add(_arcTranslation.multiplyScalar(_arcSign)).normalize();

                console.log({_startPos, _arcSign, _linearTranslation, _arcTranslation, _translationDir});

                // Scale to midsphere length
                this.postTrans = _translationDir.clone().multiplyScalar(this.midsphere);
                this.preTrans = this.postTrans.clone().negate();

                // 
                this.maxAngle = THREE.MathUtils.degToRad(deltaPos.toArray().reduce((p,a)=>p+Math.abs(a), 0.0) * this.dihedralAngle);
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

/* 
 * Module class
 * Modules contain their position, color, ID, etc. info,
 *  as well as their mesh
*/

import * as THREE from 'three';
import { ModuleType, MoveType } from "./utils.js";
import { ModuleGeometries } from "./ModuleGeometries.js";
import { gScene, gModules } from "./main.js";
import { Move } from "./Move.js"

function _createModuleMesh(moduleType, color = 0x808080, scale = 1.0) {
    let geometry = ModuleGeometries.get(moduleType);
    let material = new THREE.MeshPhongMaterial({flatShading: true, color: color});
    let mesh = new THREE.Mesh(geometry, material);
    mesh.scale.set(scale, scale, scale);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    mesh.matrixAutoUpdate = false; // We will calculate transformation matrices ourselves
    return mesh;
}

export class Module {
    constructor(moduleType, id, pos, color = 0x808080, scale = 1.0) {
        this.moduleType = moduleType;
        this.id = id;
        this.pos = pos;
        this.color = color;
        this.scale = scale;

        this.mesh = _createModuleMesh(moduleType, color, scale);
        this._setMeshMatrix();
        this.parentMesh = new THREE.Object3D(); // Parent object will never rotate
        this.parentMesh.position.set(...pos);
        this.parentMesh.add(this.mesh);
        gScene.add(this.parentMesh);
        gModules[id] = this;
    }

    _setMeshMatrix(optionalPreTransform = new THREE.Matrix4()) {
        let transform = new THREE.Matrix4().makeScale(this.scale, this.scale, this.scale).premultiply(optionalPreTransform);
        this.mesh.matrix.copy(transform);
    }

    animateMove(move, pct) {
        switch (move.moveType) {
            case MoveType.PIVOT: this._pivotAnimate(move, pct); break;
            case MoveType.SLIDING: this._slidingAnimate(move, pct); break;
            case MoveType.MONKEY: this._monkeyAnimate(move, pct); break;
        }
    }

    finishMove(move) {
        this._setMeshMatrix();
        this.parentMesh.position.add(move.deltaPos);
    }

    _pivotAnimate(move, pct) {
        let trans1 = new THREE.Matrix4().makeTranslation(move.preTrans);
        let rotate = new THREE.Matrix4().makeRotationAxis(move.rotAxis, move.maxAngle * pct);
        let trans2 = new THREE.Matrix4().makeTranslation(move.postTrans);
        let transform = new THREE.Matrix4().premultiply(trans2).premultiply(rotate).premultiply(trans1);
        this._setMeshMatrix(transform);
    }
}

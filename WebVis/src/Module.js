/* 
 * Module class
 * Modules contain their position, color, ID, etc. info,
 *  as well as their mesh
*/

import * as THREE from 'three';
import { ModuleType, MoveType } from "./utils.js";
import { ModuleGeometries } from "./ModuleGeometries.js";
import { ModuleMaterialConstructors } from "./ModuleMaterials.js";
import { gScene, gModules, gRenderer } from "./main.js";
import { Move } from "./Move.js"

const gTexLoader = new THREE.TextureLoader();
let cubeTexture = gTexLoader.load('./resources/textures/cube_face.png');
let rdTexture = gTexLoader.load('./resources/textures/cube_face.png');

function _createModuleMesh(moduleType, color = 0x808080, scale = 1.0) {
    let geometry = ModuleGeometries.get(moduleType);
    let material;
    let materialConstructor = ModuleMaterialConstructors.get(moduleType);
    let texture = moduleType == ModuleType.CUBE ? cubeTexture : rdTexture;
    switch (moduleType) {
        case ModuleType.CUBE: /* fallthrough */
        case ModuleType.RHOMBIC_DODECAHEDRON: material = materialConstructor(texture, color); break;
    }

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

        this.cumulativeRotationMatrix = new THREE.Matrix4();

        this.mesh = _createModuleMesh(moduleType, color, scale);
        this._setMeshMatrix();
        this.parentMesh = new THREE.Object3D(); // Parent object will never rotate
        this.parentMesh.position.set(...pos);
        this.parentMesh.add(this.mesh);
        let axesHelper = new THREE.AxesHelper(1.2);
        this.parentMesh.add(axesHelper);
        gScene.add(this.parentMesh);
        gModules[id] = this;
    }

    destroy() {
        gScene.remove(this.parentMesh);
        this.mesh.geometry.dispose();
        this.mesh.material.dispose();
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
        if (move.moveType !== MoveType.SLIDING) {
            let rotate = new THREE.Matrix4().makeRotationAxis(move.rotAxis, move.maxAngle);
            this.cumulativeRotationMatrix = this.cumulativeRotationMatrix.premultiply(rotate);
        }
        this._setMeshMatrix(this.cumulativeRotationMatrix);
        this.parentMesh.position.add(move.deltaPos);
        console.log(move);
    }

    _pivotAnimate(move, pct) {
        let trans1 = new THREE.Matrix4().makeTranslation(move.preTrans);
        let rotate = new THREE.Matrix4().makeRotationAxis(move.rotAxis, move.maxAngle * pct);
        let trans2 = new THREE.Matrix4().makeTranslation(move.postTrans);
        let transform = new THREE.Matrix4().premultiply(trans1).premultiply(rotate).premultiply(trans2);
        this._setMeshMatrix(transform.multiply(this.cumulativeRotationMatrix));
    }

    _slidingAnimate(move, pct) {
        let translate = new THREE.Vector3(0.0, 0.0, 0.0);
        if ((move.deltaPos.abs().sum() > 1.0) && (move.anchorDir.abs().sum() > 0.1)) {
            let _pct1, _pct2;
            let testVec = new THREE.Vector3(1.0, 1.0, 1.0);
            _pct1 = Math.min(pct * 2.0, 1.0);
            _pct2 = 1.0 - Math.min(2.0 * (1.0 - pct), 1.0);
            translate.add(move.deltaPos.clone().multiply(move.anchorDir).multiplyScalar(_pct1));
            translate.add(move.deltaPos.clone().multiply(testVec.sub(move.anchorDir)).multiplyScalar(_pct2));
        } else {
            translate = move.deltaPos.clone().multiplyScalar(pct);
        }

        let transform = new THREE.Matrix4().makeTranslation(translate);
        this._setMeshMatrix(transform);
    }
}

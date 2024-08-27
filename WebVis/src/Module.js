/* 
 * Module class
 * Modules contain their position, color, ID, etc. info,
 *  as well as their mesh
*/

import * as THREE from 'three';
import { ModuleType, MoveType } from "./utils.js";
import { ModuleGeometries } from "./ModuleGeometries.js";
import { gScene, gModules, gRenderer } from "./main.js";
import { Move } from "./Move.js"

const gTexLoader = new THREE.TextureLoader();
let cubeTexture = gTexLoader.load('./resources/textures/cube_face.png');
let rdTexture = gTexLoader.load('./resources/textures/cube_face.png');

function _createModuleMesh(moduleType, color = 0x808080, scale = 1.0) {
    let geometry = ModuleGeometries.get(moduleType);
    let material = new THREE.MeshPhongMaterial({
        flatShading: true,
        map: cubeTexture,
        color: color,
        onBeforeCompile: shader => { // Manually update the existing material shader to add borders to modules
            // Extract the index of the start of main
            //  We will declare some helper functions right before main()
            let beginningOfMain = shader.fragmentShader.indexOf('void main() {');

            // Extract the index of the closing curly brace, hack-ily
            //  We will inject code to the end of main
            let endOfMain = shader.fragmentShader.lastIndexOf('}');

            // We will inject a uniform variable at the head of the shader source file
            let uniformsToInject = 
`uniform vec4 borderAttrs;
`           // Hook up these uniforms to a variable TODO: add actual variability
            shader.uniforms.borderAttrs = { value: new THREE.Vector4(0.0, 0.0, 0.0, 0.02) }; // R,G,B,width
            
            // We will inject some helper functions
            let helperFunctions =
`float Between(float low, float high, float val) {
	return step(low, val) - step(high, val);
}
float Rectangle(vec2 orig, vec2 wh, vec2 st) {
	float x = Between(orig.x, orig.x + wh.x, st.x);
	float y = Between(orig.y, orig.y + wh.y, st.y);
	return x*y;
}
`;          // We will inject code at the end of main()
            let codeToInjectToMain =
`float borderMask = 1.0 - Rectangle(vec2(borderAttrs.w), vec2(1.0 - 2.0*borderAttrs.w), vMapUv);
float interiorMask = 1.0 - borderMask;
vec3 borderColor = borderAttrs.xyz;
vec3 border = borderMask * borderColor;

vec3 interior = gl_FragColor.xyz * interiorMask;
gl_FragColor = vec4(interior + border, 1.0);
`;          // Perform the injection
            shader.fragmentShader = 
                uniformsToInject
                + shader.fragmentShader.substring(0, beginningOfMain)
                + helperFunctions
                + shader.fragmentShader.substring(beginningOfMain, endOfMain)
                + codeToInjectToMain
                + shader.fragmentShader.substring(endOfMain);
        }
    });

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
        let rotate = new THREE.Matrix4().makeRotationAxis(move.rotAxis, move.maxAngle);
        this.cumulativeRotationMatrix = this.cumulativeRotationMatrix.premultiply(rotate);
        this._setMeshMatrix(this.cumulativeRotationMatrix);
        this.parentMesh.position.add(move.deltaPos);
    }

    _pivotAnimate(move, pct) {
        let trans1 = new THREE.Matrix4().makeTranslation(move.preTrans);
        let rotate = new THREE.Matrix4().makeRotationAxis(move.rotAxis, move.maxAngle * pct);
        let trans2 = new THREE.Matrix4().makeTranslation(move.postTrans);
        let transform = new THREE.Matrix4().premultiply(trans2).premultiply(rotate).premultiply(trans1);
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

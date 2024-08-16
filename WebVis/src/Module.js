/* 
 * Module class
 * Modules contain their position, color, ID, etc. info,
 *  as well as their mesh
*/

import * as THREE from 'three';
import { Vec3, ModuleType } from "./utils.js";
import { ModuleGeometries } from "./ModuleGeometries.js";
import { gScene } from "./main.js";

function _createModuleMesh(moduleType, pos = new Vec3(), color = 0x808080, scale = 1.0) {
    let geometry = ModuleGeometries.get(moduleType);
    let material = new THREE.MeshPhongMaterial({flatShading: true, color: color});
    let mesh = new THREE.Mesh(geometry, material);
    mesh.position.set(pos.x, pos.y, pos.z);
    mesh.scale.set(scale, scale, scale);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    return mesh;
}

export class Module {
    constructor(moduleType, id, v3Pos, color = 0x808080, scale = 1.0) {
        this.moduleType = moduleType;
        this.id = id;
        this.pos = v3Pos;
        this.color = color;
        this.scale = scale;

        this.mesh = _createModuleMesh(moduleType, v3Pos, color, scale);
        gScene.add(this.mesh);
    }
}

import * as THREE from 'three';
import { Module } from "./Module.js";
import { User } from "./User.js";
import { ModuleType, MoveType } from "./utils.js";
import { Move } from "./Move.js";
import { MoveSequence } from "./MoveSequence.js";
import { gGui } from "./GUI.js";

// Extends THREE Vector3 type with new component-wise abs() method
// TODO put this in a better place?
THREE.Vector3.prototype.abs = function() {
    return new THREE.Vector3(Math.abs(this.x), Math.abs(this.y), Math.abs(this.z));
}

/* --- setup --- */
export const gCanvas = document.getElementById("scene");
export const gRenderer = new THREE.WebGLRenderer({canvas: gCanvas});
export const gScene = new THREE.Scene();
export const gUser = new User();
export const gTexLoader = new THREE.TextureLoader();
gRenderer.setSize( window.innerWidth, window.innerHeight );
gRenderer.shadowMap.enabled = true;
gRenderer.setAnimationLoop( animate );

// Following are global attributes set directly to the window object
//  This allows them to be added to the GUI,
//  or directly modified by other scripts
window.gwForward = true;
window.gwNextAnimationRequested = false;
window.gwAnimSpeed = 1.0;

/* --- objects --- */
export const gModules = {};
[
    [123, 0.0, 0.0, 0.0],
    [124, -1.0, 0.0, 0.0],
].map( ([id, x, y, z]) => new Module(ModuleType.CUBE, id, new THREE.Vector3(x, y, z), 0x808080, 0.9))

const rhombicDodecahedron = new Module(ModuleType.RHOMBIC_DODECAHEDRON, 0, new THREE.Vector3(3.0, -1.0, -3.0));

/* --- lights --- */
const light0 = new THREE.AmbientLight(0xFFFFFF, 0.10);
const light1 = new THREE.PointLight(0x50C0FF, 100.0, 30.0);
const light2 = new THREE.PointLight(0xFFC050, 100.0, 30.0);
light1.castShadow = true;
light2.castShadow = true;
gScene.add(light0);
gScene.add(light1);
gScene.add(light2);
const helper1 = new THREE.PointLightHelper(light1, 0.25);
const helper2 = new THREE.PointLightHelper(light2, 0.25);
gScene.add(helper1);
gScene.add(helper2);

/* --- floor --- */
const planeSize = 1000;
const texture = gTexLoader.load('resources/textures/ground.png');
texture.wrapS = THREE.RepeatWrapping;
texture.wrapT = THREE.RepeatWrapping;
texture.magFilter = THREE.NearestFilter;
texture.colorSpace = THREE.SRGBColorSpace;
texture.repeat.set(planeSize/2, planeSize/2);
const planeGeo = new THREE.PlaneGeometry(planeSize, planeSize);
const planeMat = new THREE.MeshPhongMaterial({
  map: texture,
  emissive: 0x440000,
  side: THREE.DoubleSide,
});
const planeMesh = new THREE.Mesh(planeGeo, planeMat);
planeMesh.rotation.x = Math.PI * -.5;
planeMesh.position.y = -3.0;
planeMesh.receiveShadow = true;
gScene.add(planeMesh); 

let currentAnimProgress = 0.0; // 0.0-1.0
let moves = [
    new Move(123, new THREE.Vector3(-1.0, 0.0, 0.0), new THREE.Vector3(-1.0, 1.0, 0.0), MoveType.PIVOT, true),
    new Move(123, new THREE.Vector3(0.0, -1.0, 0.0), new THREE.Vector3(-1.0, -1.0, 0.0), MoveType.PIVOT, false),
    new Move(123, new THREE.Vector3(1.0, 0.0, 0.0), new THREE.Vector3(1.0, 0.0, 1.0), MoveType.PIVOT, true),
]

// TODO Put all this in a better place
let moveSequence = new MoveSequence(moves);
let move;
let gDeltaTime;
let readyForNewAnimation = true;

let lastFrameTime = 0;
function animate(time) {
    gDeltaTime = time - lastFrameTime;
    lastFrameTime = time;

    rhombicDodecahedron.mesh.rotation.x += 0.0015;
    light1.position.set(Math.cos(time / 3000.0) * 1.5, 3.0, Math.sin(time / 3000.0) * 1.5);
    light2.position.set(Math.cos(time / 3000.0 - 3.14) * 1.5, 3.0, Math.sin(time / 3000.0 - 3.14) * 1.5);

    if (currentAnimProgress > 1.0) { // Wrap up current animation if needed
        gModules[move.id].finishMove(move);
        readyForNewAnimation = true;
        currentAnimProgress = 0.0;
    }

    if (readyForNewAnimation && window.gwNextAnimationRequested) { // Fetch and start new animation if needed
        move = window.gwForward ? moveSequence.pop() : moveSequence.undo();
        if (move) { readyForNewAnimation = false; }
    }

    if (move) { // Perform animation (if there's one active)
        gModules[move.id].animateMove(move, currentAnimProgress);
        currentAnimProgress += gDeltaTime * gAnimSpeed / 1000.0;
    }

	gRenderer.render( gScene, gUser.camera );
}

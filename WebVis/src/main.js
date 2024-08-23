import * as THREE from 'three';
import { Module } from "./Module.js";
import { User } from "./User.js";
import { ModuleType, MoveType } from "./utils.js";
import { Move } from "./Move.js";
import { MoveSequence } from "./MoveSequence.js";
import { gGui } from "./GUI.js";
import { Scenario } from "./Scenario.js";

// Extends THREE Vector3 type with new component-wise abs() and sum() methods
// TODO put this in a better place?
THREE.Vector3.prototype.abs = function() {
    return new THREE.Vector3(Math.abs(this.x), Math.abs(this.y), Math.abs(this.z));
}
THREE.Vector3.prototype.sum = function() {
    return (this.x + this.y + this.z);
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
//  This allows them to (more easily) be added to the GUI,
//  or directly modified by other scripts
window.gwAutoAnimate = false;
window.gwForward = true;
window.gwNextAnimationRequested = false;
window.gwAnimSpeed = 1.0;
window.gwUser = gUser;
window.gwMoveSequence = null;

/* --- objects --- */
export const gModules = {}; // Module constructor automatically adds modules to this

/* --- lights --- */
const light0 = new THREE.AmbientLight(0xFFFFFF, 1.0);
gScene.add(light0);

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

// TODO Put all this in a better place?
let move;
let lastFrameTime = 0;
let gDeltaTime;
let readyForNewAnimation = true;
let currentAnimProgress = 0.0; // 0.0-1.0

function animate(time) {
    gDeltaTime = time - lastFrameTime;
    lastFrameTime = time;

    if (currentAnimProgress > 1.0) { // Wrap up current animation if needed
        gModules[move.id].finishMove(move); // Offset the module
        readyForNewAnimation = true; // Flag that we can handle starting a new anim

        // If we're auto-animating, flag that we want another anim
        //  Otherwise, handle checkpoint moves:
        //      If we're animating forwards, and the next move is a checkpoint move,
        //      OR if we're animating backwards and the just-finished move was a checkpoint move,
        //          don't request another animation.
        if (window.gwAutoAnimate) {
            window.gwNextAnimationRequested = true;
        } else {
            let _move = window.gwForward ? window.gwMoveSequence.moves[0] : move;
            window.gwNextAnimationRequested = _move ? _move.checkpoint : false;
        }

        currentAnimProgress = 0.0;
        move = null;
    }

    if (readyForNewAnimation && window.gwNextAnimationRequested) { // Fetch and start new animation if needed
        move = window.gwForward ? window.gwMoveSequence.pop() : window.gwMoveSequence.undo();
        if (move) { readyForNewAnimation = false; }
    }

    if (move) { // Perform animation (if there's one active)
        gModules[move.id].animateMove(move, currentAnimProgress);
        currentAnimProgress += gDeltaTime * window.gwAnimSpeed / 1000.0;
    }

	gRenderer.render( gScene, gUser.camera );
}

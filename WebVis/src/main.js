import * as THREE from 'three';
import { SVGRenderer } from 'three/addons/renderers/SVGRenderer.js';
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
gCanvas.width = 0;
gCanvas.height = 0;
//export const gRenderer = new THREE.WebGLRenderer({canvas: gCanvas});
//gRenderer.shadowMap.enabled = true;

export const gLights = {_fullbright: false};
export const gScene = new THREE.Scene();
export const gUser = new User();

export const gRenderer = new SVGRenderer();
gRenderer.setSize( window.innerWidth, window.innerHeight );
THREE.ColorManagement.enabled = false;
document.body.appendChild( gRenderer.domElement );
requestAnimationFrame(animate);

gScene._backgroundColors = [new THREE.Color(0x334D4D), new THREE.Color(0xFFFFFF), new THREE.Color(0x000000)];
gScene._backgroundColorSelected = 0;
gScene.background = gScene._backgroundColors[gScene._backgroundColorSelected];

// Following are global attributes set directly to the window object
//  This allows them to (more easily) be added to the GUI,
//  or directly modified by other scripts
window.gwAutoAnimate = false;
window.gwForward = true;
window.gwNextAnimationRequested = false;
window.gwAnimSpeed = 1.0;
window.gwUser = gUser;
window.gwMoveSequence = new MoveSequence();

/* --- objects --- */
// Module constructor automatically adds modules to this global
export const gModules = {}
new Module(ModuleType.RHOMBIC_DODECAHEDRON, 0, new THREE.Vector3(0.0, 0.0, 0.0), 0xFFFFFF, 0.9);
new Module(ModuleType.RHOMBIC_DODECAHEDRON, 1, new THREE.Vector3(0.0, -1.0, 1.0), 0x00FF00, 0.9);
new Module(ModuleType.RHOMBIC_DODECAHEDRON, 2, new THREE.Vector3(1.0, -1.0, 0.0), 0xFF0000, 0.9);
new Module(ModuleType.RHOMBIC_DODECAHEDRON, 3, new THREE.Vector3(0.0, -1.0, -1.0), 0x00FF00, 0.9);
new Module(ModuleType.RHOMBIC_DODECAHEDRON, 4, new THREE.Vector3(-1.0, -1.0, 0.0), 0xFF0000, 0.9);

/* --- lights --- */
export const lightAmbient = new THREE.AmbientLight(0xFFFFFF, 0.00);
const lightDirectional = new THREE.DirectionalLight(0xFFFFFF, 0.7);
lightDirectional.position.set(1, 1, 1);
gScene.add(lightAmbient);
gScene.add(lightDirectional);
gLights.lightAmbient = lightAmbient;
gLights.lightDirectional = lightDirectional;
gLights._defaultAmbientIntensity = lightAmbient.intensity;
gLights._defaultDirectionalIntensity = lightDirectional.intensity;

/* --- debug --- */
let axesHelper = new THREE.AxesHelper(5);
gScene.add(axesHelper);

// TODO Put all this in a better place?
let move;
let lastFrameTime = 0;
let gDeltaTime;
let readyForNewAnimation = true;
let currentAnimProgress = 0.0; // 0.0-1.0

let debugCount = 2;

export function cancelActiveMove() {
    move = null;
    currentAnimProgress = 0.0;
    readyForNewAnimation = true;
    window.gwNextAnimationRequested = false;
}

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
            window.gwNextAnimationRequested = _move ? !_move.checkpoint : false;
        }

        currentAnimProgress = 0.0;
        move = null;
    }

    if (readyForNewAnimation && window.gwNextAnimationRequested) { // Fetch and start new animation if needed
        move = window.gwForward ? window.gwMoveSequence.pop() : window.gwMoveSequence.undo();
        // TODO could add some effects to show which cube is moving --
        //  e.g. attaching a light to the mover,
        //  changing its material, etc
        if (move) { readyForNewAnimation = false; }
    }

    if (move) { // Perform animation (if there's one active)
        gModules[move.id].animateMove(move, currentAnimProgress);
        currentAnimProgress += gDeltaTime * window.gwAnimSpeed / 1000.0;
    }

    gUser.controls.update();

	gRenderer.render(gScene, gUser.camera);

    if (debugCount > 0) {
        debugCount--;
    }

    requestAnimationFrame(animate);
}

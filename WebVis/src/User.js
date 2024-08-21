/* 
 * User class
 * User is a wrapper around the three.js Camera observing the scene,
 *  containing view settings (projection type, zoom level, etc)
 *  as well as methods to switch view settings
 * This module also handles other types of user input
 *  (such as window resizing)
*/

import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { gCanvas, gUser, gRenderer } from "./main.js";
import { CameraType } from "./utils.js";

export class User {
    constructor() {
        this.cameraStyle = CameraType.PERSPECTIVE;
        this.resetCamera();
    }

    resetCamera() {
        let newCamera;
        switch (this.cameraStyle) {
            case CameraType.PERSPECTIVE: newCamera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000.0 ); break;
            case CameraType.ORTHOGRAPHIC: {
                let width = window.innerWidth / 200;
                let height = window.innerHeight / 200;
                newCamera = new THREE.OrthographicCamera( -width, width, height, -height, 0.1, 1000.0 ); break;
            }
        }
        newCamera.position.z = 5.0;
        this.controls = new OrbitControls(newCamera, gCanvas);
        this.camera = newCamera;
    }

    toggleCameraStyle() {
        let newCameraStyle;
        switch(this.cameraStyle) {
            case CameraType.PERSPECTIVE: newCameraStyle = CameraType.ORTHOGRAPHIC; break;
            case CameraType.ORTHOGRAPHIC: newCameraStyle = CameraType.PERSPECTIVE; break;
        }
        this.cameraStyle = newCameraStyle;
        this.resetCamera();
        console.log(newCameraStyle);
    }
}

function window_resize_callback() {
    const width = window.innerWidth;
    const height = window.innerHeight;

    let newAspect = width/height;
    switch (gUser.cameraStyle) {
        case CameraType.PERSPECTIVE: gUser.camera.aspect = newAspect; break;
        case CameraType.ORTHOGRAPHIC: {
            let oldAspect = gUser.camera.right / gUser.camera.top;
            gUser.camera.left = gUser.camera.left * (newAspect / oldAspect);
            gUser.camera.right = gUser.camera.right * (newAspect / oldAspect);
            break;
        }
    }

    gUser.camera.updateProjectionMatrix();
    gRenderer.setSize(width, height);
}

function keydown_input_callback(event) {
    let key = event.key;
    console.log(key);
    switch (key) {
        case 'p': gUser.toggleCameraStyle(); break;
        case 'r': gUser.resetCamera(); break;
        case 'ArrowRight': window.gwNextAnimationRequested = true; window.gwForward = true; break;
        case 'ArrowLeft': window.gwNextAnimationRequested = true; window.gwForward = false; break;
        default: break;
    }
}

window.addEventListener('resize', window_resize_callback);
window.addEventListener('keydown', keydown_input_callback);


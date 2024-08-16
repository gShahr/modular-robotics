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

export class User {
    constructor() {
        this.camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
        this.controls = new OrbitControls(this.camera, gCanvas);
        this.controls.update();
        this.camera.position.z = 5;
    }
}

function window_resize_callback() {
    const width = window.innerWidth;
    const height = window.innerHeight;
    gRenderer.setSize(width, height);
    gUser.camera.aspect = width/height;
    gUser.camera.updateProjectionMatrix();
}
window.addEventListener('resize', window_resize_callback);


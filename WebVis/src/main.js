import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

function window_resize_callback() {
    const width = window.innerWidth;
    const height = window.innerHeight;
    renderer.setSize(width, height);
}

class User {
    constructor() {
        this.camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
        this.controls = new OrbitControls(this.camera, canvas);
        this.controls.update();
        this.camera.position.z = 5;
    }
}

const cubeGeometry = new THREE.BoxGeometry( 1, 1, 1 );
class Cube {
    constructor(x, y, z, color = 0x555555, scale = 1.0) {
        this.x = x;
        this.y = y;
        this.z = z;
        let material = new THREE.MeshPhongMaterial( { color: color } );
        this.mesh = new THREE.Mesh( cubeGeometry, material );
        this.mesh.position.set(x, y, z);
        this.mesh.scale.set(scale, scale, scale);
        this.mesh.castShadow = true;
        this.mesh.receiveShadow = true;
    }
}

const rhombicDodecahedronGeometryVertices = new Float32Array([
   -1.0, -1.0, -1.0, //  0 | A -- Obtuse vertices / Cube
   -1.0, -1.0,  1.0, //  1 | B
    1.0, -1.0,  1.0, //  2 | C
    1.0, -1.0, -1.0, //  3 | D
   -1.0,  1.0, -1.0, //  4 | E
   -1.0,  1.0,  1.0, //  5 | F
    1.0,  1.0,  1.0, //  6 | G
    1.0,  1.0, -1.0, //  7 | H

    0.0,  0.0,  2.0, //  8 | J -- Acute vertices // pyramid tips
   -2.0,  0.0,  0.0, //  9 | K
    2.0,  0.0,  0.0, // 10 | L
    0.0, -2.0,  0.0, // 11 | M
    0.0,  2.0,  0.0, // 12 | N
    0.0,  0.0, -2.0, // 13 | O
].map((x) => x / 2.0));

// faces - in counterclockwise winding order - important!
const rhombicDodecahedronGeometryIndices = [
    12,  4,  5,   5,  4,  9, // NEKF -- NEF / FEK
    12,  5,  6,   6,  5,  8, // NFJG -- NFG / GFJ
    12,  6,  7,   7,  6, 10, // NGLH -- NGH / HGL
    12,  7,  4,   4,  7, 13, // NHOE -- NHE / EHO
    11,  1,  0,   0,  1,  9, // MAKB -- MBA / ABK
    11,  2,  1,   1,  2,  8, // MBJC -- MCB / BCJ
    11,  3,  2,   2,  3, 10, // MCLD -- MDC / CDL
    11,  0,  3,   3,  0, 13, // MDOA -- MAD / DAO

     5,  9,  8,   8,  9,  1, // FKBJ -- FKJ / JKB
     6,  8, 10,  10,  8,  2, // GJCL -- GJL / LJC
     7, 10, 13,  13, 10,  3, // HLDO -- HLO / OLD
     4, 13,  9,   9, 13,  0  // EKAO -- EKO / OKA
];
const rhombicDodecahedronGeometry = new THREE.BufferGeometry();
rhombicDodecahedronGeometry.setIndex(rhombicDodecahedronGeometryIndices);
rhombicDodecahedronGeometry.setAttribute('position', new THREE.BufferAttribute(rhombicDodecahedronGeometryVertices, 3));
rhombicDodecahedronGeometry.computeVertexNormals();

class RhombicDodecahedron {
    constructor(x, y, z, color = 0x555555, scale = 1.0) {
        this.x = x;
        this.y = y;
        this.z = z;
        let material = new THREE.MeshPhongMaterial( { color: color } );
        this.mesh = new THREE.Mesh( rhombicDodecahedronGeometry, material );
        this.mesh.position.set(x, y, z);
        this.mesh.scale.set(scale, scale, scale);
        this.mesh.castShadow = true;
        this.mesh.receiveShadow = true;
    }
}


/* --- setup --- */
const canvas = document.getElementById("scene");
const renderer = new THREE.WebGLRenderer({canvas: canvas});
const scene = new THREE.Scene();
const user = new User();
const loader = new THREE.TextureLoader();
renderer.setSize( window.innerWidth, window.innerHeight );
renderer.setAnimationLoop( animate );
renderer.shadowMap.enabled = true;

window.addEventListener('resize', window_resize_callback);

/* --- lights --- */
const light0 = new THREE.AmbientLight(0xFFFFFF, 0.6);
const light1 = new THREE.PointLight(0xFF0000, 500.0);
light1.position.set(5.0, 3.0, 2.0);
light1.castShadow = true;
scene.add(light0);
scene.add(light1);
const helper = new THREE.PointLightHelper(light1);
scene.add(helper);

/* --- objects --- */
const cubes = [-2.0, 2.0].map( (x) => new Cube(x, 0.0, 0.0))
cubes.forEach((c) => scene.add(c.mesh));
const rhombicDodecahedron = new RhombicDodecahedron(0.0, 1.0, -1.0);
scene.add(rhombicDodecahedron.mesh);

/* --- floor --- */
const planeSize = 400;
const texture = loader.load('resources/textures/ground.png');
texture.wrapS = THREE.RepeatWrapping;
texture.wrapT = THREE.RepeatWrapping;
texture.magFilter = THREE.NearestFilter;
texture.colorSpace = THREE.SRGBColorSpace;
texture.repeat.set(planeSize/2, planeSize/2);
const planeGeo = new THREE.PlaneGeometry(planeSize, planeSize);
const planeMat = new THREE.MeshPhongMaterial({
  map: texture,
  side: THREE.DoubleSide,
});
const planeMesh = new THREE.Mesh(planeGeo, planeMat);
planeMesh.rotation.x = Math.PI * -.5;
planeMesh.position.y = -3.0;
planeMesh.receiveShadow = true;
scene.add(planeMesh); 

function animate(time) {
	renderer.render( scene, user.camera );
    cubes.forEach((c) => { c.mesh.rotation.x += 0.01; c.mesh.rotation.y += 0.01; } );
    rhombicDodecahedron.mesh.rotation.x += 0.01;
    rhombicDodecahedron.mesh.rotation.y += 0.02;
    light1.position.set(Math.cos(time / 1000.0) * 3.0, 3.0, Math.sin(time / 1000.0) * 3.0);

    user.controls.update();
}

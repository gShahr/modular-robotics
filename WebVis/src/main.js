import * as THREE from 'three';
import { Module } from "./Module.js";
import { User } from "./User.js";
import { ModuleType } from "./utils.js";
import { Move } from "./Move.js";

/* --- setup --- */
export const gCanvas = document.getElementById("scene");
export const gRenderer = new THREE.WebGLRenderer({canvas: gCanvas});
export const gScene = new THREE.Scene();
export const gUser = new User();
export const gTexLoader = new THREE.TextureLoader();
gRenderer.setSize( window.innerWidth, window.innerHeight );
gRenderer.shadowMap.enabled = true;
gRenderer.setAnimationLoop( animate );

/* --- objects --- */
const cubes = [
    [-2.0, 0.0, 0.0],
    [-2.0, 1.0, 0.0],
    [-3.0, 0.0, 0.0],
].map( ([x, y, z]) => new Module(ModuleType.CUBE, 0, new THREE.Vector3(x, y, z), 0x808080, 0.9))

const rhombicDodecahedron = new Module(ModuleType.RHOMBIC_DODECAHEDRON, 0, new THREE.Vector3(0.0, 1.0, -1.0));

/* --- lights --- */
const light0 = new THREE.AmbientLight(0xFFFFFF, 0.05);
const light1 = new THREE.PointLight(0x50C0FF, 500.0, 7.0);
light1.position.set(5.0, 3.0, 2.0);
light1.castShadow = true;
gScene.add(light0);
gScene.add(light1);
const helper = new THREE.PointLightHelper(light1, 0.25);
gScene.add(helper);

/* --- floor --- */
const planeSize = 20;
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

const move = new Move(0, new THREE.Vector3(0.0, 1.0, 0.0), new THREE.Vector3(-1.0, 1.0, 0.0), false);
function animate(time) {
    cubes[0].pivotAnimate(move, Math.abs(Math.sin(time / 1000.0)));
    rhombicDodecahedron.mesh.rotation.x += 0.0015;
    light1.position.set(Math.cos(time / 3000.0) * 3.0, 3.0, Math.sin(time / 3000.0) * 3.0);

    gUser.controls.update();
	gRenderer.render( gScene, gUser.camera );
}

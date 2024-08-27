/* Module geometry data */

import * as THREE from 'three';
import { ModuleType } from "./utils.js";

const _cubeGeometry = new THREE.BoxGeometry( 1, 1, 1 );

// TODO add uv
const _rhombicDodecahedronGeometryVertices = new Float32Array([
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
const _rhombicDodecahedronGeometryIndices = [
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
const _rhombicDodecahedronGeometry = new THREE.BufferGeometry();
_rhombicDodecahedronGeometry.setIndex(_rhombicDodecahedronGeometryIndices);
_rhombicDodecahedronGeometry.setAttribute('position', new THREE.BufferAttribute(_rhombicDodecahedronGeometryVertices, 3));
_rhombicDodecahedronGeometry.computeVertexNormals();

export const ModuleGeometries = new Map([
    [ModuleType.CUBE, _cubeGeometry],
    [ModuleType.RHOMBIC_DODECAHEDRON, _rhombicDodecahedronGeometry]
]);

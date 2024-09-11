import * as THREE from 'three';
import { ModuleType, MoveType } from "./utils.js";
import { Module } from "./Module.js";
import { Move } from "./Move.js";
import { MoveSequence } from "./MoveSequence.js";
import { gModules, gRenderer } from "./main.js";

function Visgroup(r, g, b, scale) {
    this.color = `rgb(${r}, ${g}, ${b})`;
    this.scale = scale / 100;
}

// TODO this doesn't really need to be a class, with the way the code is structured
//  modules are loaded globally into gModules,
//  and moves/MoveSequence is loaded globally into window.gwMoveSequence
export class Scenario {
    constructor(rawString) {
        for (let module in gModules) gModules[module].destroy();

        let visgroups = {}; // key-value pairs of 'visgroupId: visgroup'
        let lines = rawString.split('\n');
        let nBlock = 0;
        let checkpointMove = true;
        let moves = [];
        for (let iLine = 0; iLine < lines.length; iLine++) {

            // sanitize the line: remove spaces, \r characters, and comments
            let line = lines[iLine].replace(/ /g, '').replace(/\r/g, '').split("//")[0];

            // if the line is empty, skip it and increment our block counter
            //  also, flag the next move as a checkpoint move
            //      (will only be relevant if we're on block 2+)
            if (!line) { nBlock++; checkpointMove = true; continue; }

            // extract the individual values from the line
            let lineVals = line.split(',').map((val) => parseInt(val));

            // perform different logic depending on which block we're in
            switch (nBlock) {
                case 0: { // Visgroup definitions
                    let vgId = lineVals[0];
                    let r = lineVals[1];
                    let g = lineVals[2];
                    let b = lineVals[3];
                    let scale = lineVals[4];
                    visgroups[vgId] = new Visgroup(r, g, b, scale);
                    break;
                }
                case 1: { // Module definitions
                    let moduleId = lineVals[0];
                    let vg = visgroups[lineVals[1]];
                    let pos = new THREE.Vector3(lineVals[2], lineVals[3], lineVals[4]);
                    new Module(ModuleType.CUBE, moduleId, pos, vg.color, vg.scale);
                    break;
                }
                default: { // Move definitions
                    let moverId = lineVals[0];
                    let anchorDirCode = lineVals[1];
                    let deltaPos = new THREE.Vector3(lineVals[2], lineVals[3], lineVals[4]);
                    // TODO if we add more move types, this needs to be changed
                    let moveType = anchorDirCode > 0 ? MoveType.PIVOT : MoveType.SLIDING;
                    let anchorDir;
                    switch (Math.abs(anchorDirCode)) {
                        // Generic sliding move
                        case 0:   anchorDir = new THREE.Vector3( 0.0,  0.0,  0.0 ); break; // generic slide

                        // Cube pivots
                        case 1:   anchorDir = new THREE.Vector3( 1.0,  0.0,  0.0 ); break; // +x
                        case 2:   anchorDir = new THREE.Vector3( 0.0,  1.0,  0.0 ); break; // +y
                        case 3:   anchorDir = new THREE.Vector3( 0.0,  0.0,  1.0 ); break; // +z
                        case 4:   anchorDir = new THREE.Vector3(-1.0,  0.0,  0.0 ); break; // -x
                        case 5:   anchorDir = new THREE.Vector3( 0.0, -1.0,  0.0 ); break; // -y
                        case 6:   anchorDir = new THREE.Vector3( 0.0,  0.0, -1.0 ); break; // -z

                        // Rhombic dodecahedron: pivots away from faces which have normals in the xy plane
                        case 12:  anchorDir = new THREE.Vector3( 1.0,  1.0,  0.0 ); break; // +x +y
                        case 15:  anchorDir = new THREE.Vector3( 1.0, -1.0,  0.0 ); break; // +x -y
                        case 42:  anchorDir = new THREE.Vector3(-1.0,  1.0,  0.0 ); break; // -x +y
                        case 45:  anchorDir = new THREE.Vector3(-1.0, -1.0,  0.0 ); break; // -x -y

                        // Rhombic dodecahedron: pivots away from faces which have normals in an octant of xyz space
                        case 123: anchorDir = new THREE.Vector3( 1.0,  1.0,  1.0 ); break; // +x +y +z
                        case 126: anchorDir = new THREE.Vector3( 1.0,  1.0, -1.0 ); break; // +x +y -z
                        case 153: anchorDir = new THREE.Vector3( 1.0, -1.0,  1.0 ); break; // +x -y +z
                        case 156: anchorDir = new THREE.Vector3( 1.0, -1.0, -1.0 ); break; // +x -y -z
                        case 423: anchorDir = new THREE.Vector3(-1.0,  1.0,  1.0 ); break; // -x +y +z
                        case 426: anchorDir = new THREE.Vector3(-1.0,  1.0, -1.0 ); break; // -x +y -z
                        case 453: anchorDir = new THREE.Vector3(-1.0, -1.0,  1.0 ); break; // -x -y +z
                        case 456: anchorDir = new THREE.Vector3(-1.0, -1.0, -1.0 ); break; // -x -y -z

                        default:  anchorDir = new THREE.Vector3( 0.0,  0.0,  0.0 ); console.log("Unknown rotation code ", anchorDirCode, " -- treating as sliding move"); break;
                    }
                    anchorDir.normalize();
                    moves.push(new Move(moverId, anchorDir, deltaPos, moveType, checkpointMove));
                    if (checkpointMove) { checkpointMove = false; }
                    break;
                }
            }  // end Switch statement
        } // end For loop (line iteration)

        window.gwMoveSequence = new MoveSequence(moves);
    } // end Constructor
}


const scenarioUploadElement = document.getElementById("scenarioUploadButton");
scenarioUploadElement.onchange = (e) => {
    const file = scenarioUploadElement.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (e) => {
        const textContent = e.target.result;
        new Scenario(textContent);
    }
    reader.onerror = (e) => {
        const error = e.target.error;
    }
    reader.readAsText(file);
}


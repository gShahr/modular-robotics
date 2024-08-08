document.oncontextmenu = () => { return false; }
canvasW = window.screen.width * .99;
canvasH = window.screen.height * .8;
canvasZ = 75;
twoDtileSize = canvasW / (2 * 30);
layer = 0;
highlight = false;
objects = [];
blocks = [];
historyStack = [];
redoStack = [];
rgbColor = [255, 255, 255];

function saveConfig() {
    var output = "";
    for (i = 0; i < blocks.length; i++) {
        output += blocks[i].x + "," + blocks[i].y + ",";
    }
    dwnldAsTxt("3Dtiles.txt", output);
}

function exportToJson() {
    var jsonOutput = "{\n";
    var maxSize = 0
    for (var i = 0; i < blocks.length; i++){
        current_block = blocks[i];
	if(current_block.x > maxSize){
		maxSize = current_block.x;
	}
	if(current_block.y > maxSize){
		maxSize = current_block.y;
	}
	if(current_block.z > maxSize){
		maxSize = current_block.z;
	}
    }
    maxSize = maxSize + 1;
    jsonOutput += "    \"order\": 3,\n";
    jsonOutput += "    \"axisSize\": " + maxSize + ",\n";
    jsonOutput += "    \"modules\": [";
    for (var i = 0; i < blocks.length; i++){
        current_block = blocks[i];
        jsonOutput += "\n{\n	\"position\": [" + current_block.x + ", " + current_block.y + ", " + current_block.z + "],\n"
        jsonOutput += "	\"static\": true\n}"
        if (i < blocks.length - 1) {
            jsonOutput += ",";
        }
    }
    dwnldAsTxt("3Dtiles.json", jsonOutput);
}

function exportToScen() {
    var ScenOutput = "";
    ScenOutput += "0 244 244 0 100\n\n"
    for (var i=0; i < blocks.length; i++){
	if(i < 10){
		ScenOutput += "0";
	}
	current_block = blocks[i];
	ScenOutput += i + "," + "0" + "," + current_block.x + "," + current_block.y + "," + current_block.z + "\n";
    }
    dwnldAsTxt("3Dtiles.scen", ScenOutput);
}

function exportToObj() {
    let objData = '';
    let vertexOffset = 1;
    // let objects = [
    //     {
    //         vertices: [
    //             { x: 0, y: 0, z: 0 }, // Vertex 0
    //             { x: 1, y: 0, z: 0 }, // Vertex 1
    //             { x: 1, y: 1, z: 0 }, // Vertex 2
    //             { x: 0, y: 1, z: 0 }, // Vertex 3
    //             { x: 0, y: 0, z: 1 }, // Vertex 4
    //             { x: 1, y: 0, z: 1 }, // Vertex 5
    //             { x: 1, y: 1, z: 1 }, // Vertex 6
    //             { x: 0, y: 1, z: 1 }  // Vertex 7
    //         ],
    //         faces: [
    //             { a: 0, b: 1, c: 2 }, { a: 0, b: 2, c: 3 }, // Front face
    //             { a: 1, b: 5, c: 6 }, { a: 1, b: 6, c: 2 }, // Right face
    //             { a: 5, b: 4, c: 7 }, { a: 5, b: 7, c: 6 }, // Back face
    //             { a: 4, b: 0, c: 3 }, { a: 4, b: 3, c: 7 }, // Left face
    //             { a: 3, b: 2, c: 6 }, { a: 3, b: 6, c: 7 }, // Top face
    //             { a: 4, b: 5, c: 1 }, { a: 4, b: 1, c: 0 }  // Bottom face
    //         ]
    //     }
    // ];

    // Assuming you have an array of 3D objects, each with vertices and faces
    // objects.forEach(object => {
    //     object.vertices.forEach(vertex => {
    //         objData += `v ${vertex.x} ${vertex.y} ${vertex.z}\n`;
    //     });

    //     object.faces.forEach(face => {
    //         objData += `f ${face.a + vertexOffset} ${face.b + vertexOffset} ${face.c + vertexOffset}\n`;
    //     });

    //     vertexOffset += object.vertices.length;
    // });

    downloadObj(objects);
}

function downloadObj(data) {
    const blob = new Blob([data], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'model.obj';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
}

function dwnldAsTxt(filename, text) {
    var element = document.createElement('a');
    element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
    element.setAttribute('download', filename);
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);
}

function changeColor(color) {
    rgbColor = color;
}

function ChangeLabel(newString) {
    document.getElementById("curLayer").textContent = newString;
}

function upLayer() {
    layer++;
    ChangeLabel("Current Layer = " + layer);
}

function downLayer() {
    layer--;
    ChangeLabel("Current Layer = " + layer);
}

function highlightLayer() {
    highlight = !highlight;
}

var sketch1 = function (sketch) {
    let undoPressed = false;
    let redoPressed = false;
    let clearPressed = false;
    let switchShapePressed = false;

    sketch.setup = function () {
        canv1 = sketch.createCanvas(canvasW / 2, canvasH);
        canv1.position(0, 30);
        screen = new twoDScreen(canvasW / 2, canvasH, twoDtileSize);
        prevLayer = layer;
        let undoButton = document.getElementById('undo');
        if (undoButton) {
            undoButton.addEventListener('click', function () {
                undoPressed = true;
            });
        }
        let redoButton = document.getElementById('redo');
        if (redoButton) {
            redoButton.addEventListener('click', function () {
                redoPressed = true;
            });
        }
        let clearButton = document.getElementById('clear');
        if (clearButton) {
            clearButton.addEventListener('click', function () {
                clearPressed = true;
            });
        }
        let switchShapeButton = document.getElementById('switch-shape');
        if (switchShapeButton) {
            switchShapeButton.addEventListener('click', function () {
                switchShapePressed ^= true;
            });
        }
    }

    sketch.draw = function () {
        if (layer > prevLayer) {
            screen.upLayer();
        }
        if (layer < prevLayer) {
            screen.downLayer();
        }
        sketch.background(0, 255, 0);
        screen.draw(sketch);
        prevLayer = layer;
        switch (screen.shape) {
            case 'hexagon':
                blocks = screen.getHexagons;
                objects = threeScreen.generateHexagonsOjects();
                break;
            case 'cube':
                blocks = screen.getCubes;
                objects = threeScreen.generateCubeObjects();
                break;
            case 'rhombicDodecahedron':
                blocks = screen.getRhomdods;
                objects = threeScreen.generateRhomdodObjects();
                break;
            default:
                console.error('Unknown shape:', screen.shape);
                blocks = [];
                break;
        }
        if (undoPressed) {
            console.log("Undo Move");
            if (historyStack.length > 0) {
                let lastMove = historyStack.pop();
                redoStack.push(lastMove);
                if (lastMove.action === 'add') {
                    screen.removeCube(lastMove.x, lastMove.y, lastMove.z);
                    threeScreen.removeCube(lastMove.x, lastMove.y, lastMove.z);
                } else {
                    screen.addCube(new Cube(lastMove.x, lastMove.y, lastMove.z, lastMove.color));
                    threeScreen.addCube(new Cube(lastMove.x, lastMove.y, lastMove.z, lastMove.color));
                }
            }
            undoPressed = false;
        }
        if (redoPressed) {
            if (redoStack.length > 0) {
                console.log("Redo Move");
                let lastMove = redoStack.pop();
                historyStack.push(lastMove);
                if (lastMove.action === 'add') {
                    screen.addCube(new Cube(lastMove.x, lastMove.y, lastMove.z, lastMove.color));
                    threeScreen.addCube(new Cube(lastMove.x, lastMove.y, lastMove.z, lastMove.color));
                } else {
                    screen.removeCube(lastMove.x, lastMove.y, lastMove.z);
                    threeScreen.removeCube(lastMove.x, lastMove.y, lastMove.z);
                }
            }
            redoPressed = false;
        }
        if (clearPressed) {
            console.log("Clearing Screen");
            screen.removeAllCubes()
            threeScreen.removeAllCubes();
            clearPressed = false;
        }
        if (switchShapePressed) {
            if (document.getElementById("switch-shape").innerText === "Switch to Hexagon") {
                document.getElementById("switch-shape").innerText = "Switch to Rhombic Dodecahedron";
                screen.setShape("hexagon");
                threeScreen.setShape("hexagon");
            } else if (document.getElementById("switch-shape").innerText === "Switch to Cube") {
                document.getElementById("switch-shape").innerText = "Switch to Hexagon";
                screen.setShape("cube");
                threeScreen.setShape("cube");
            } else if (document.getElementById("switch-shape").innerText === "Switch to Rhombic Dodecahedron") {
                document.getElementById("switch-shape").innerText = "Switch to Cube";
                screen.setShape("rhombicDodecahedron");
                threeScreen.setShape("rhombicDodecahedron");
            }
            switchShapePressed = false;
            clearPressed = true;
        }
        // let message = areAllCubesConnected(blocks) ? "Yes" : "No";
        // document.getElementById("checkConnectivity").innerText = "Connected: " + message;    
    }

    sketch.mousePressed = function () {
        if (!(sketch.mouseX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0)) return;
        if (screen.shape === "cube") {
            x = Math.floor(sketch.mouseX / twoDtileSize);
            y = Math.floor(sketch.mouseY / twoDtileSize);
            if (!screen.removeCube(x, y, screen.layer)) {
                screen.addCube(new Cube(x, y, screen.layer, rgbColor));
                historyStack.push({ action: 'add', x: x, y: y, z: screen.layer, color: rgbColor });
            } else {
                historyStack.push({ action: 'remove', x: x, y: y, z: screen.layer, color: rgbColor });
            }
        } else if (screen.shape == "hexagon") {
            let [x, y] = pixelToHex(sketch.mouseX, sketch.mouseY, twoDtileSize);
            if (!screen.removeHexagon(x, y, screen.layer)) {
                screen.addHexagon(new Hexagon(x, y, screen.layer));
            } else {
            }
        } else if (screen.shape = "rhombicDodecahedron") {
            x = Math.floor(sketch.mouseX / twoDtileSize);
            y = Math.floor(sketch.mouseY / twoDtileSize);
            if (!includesArray(screen.invalidRhomdod, [x, y, screen.layer]) && !screen.removeRhomdod(x, y, screen.layer)) {
                screen.addRhomdod(new RhomDod(x, y, screen.layer));
            } else {
            }
            console.log(x, y, screen.layer);
            console.log(screen.invalidRhomdod);
        }
    }
}

var sketch2 = function (sketch) {
    sketch.setup = function () {
        canv2 = sketch.createCanvas(canvasW / 2, canvasH, sketch.WEBGL);
        canv2.position(canvasW / 2, 30);
        threeScreen = new threeDScreen(canvasW / 2, canvasH, twoDtileSize / 5);
        sketch._center = [0, 0, 0];
        sketch.createEasyCam();
    }

    sketch.draw = function () {
        sketch.background(205, 102, 94);
        threeScreen.draw(sketch, highlight, layer);
    }

    sketch.mousePressed = function () {
        if (threeScreen.shape === "rhombicDodecahedron") {
            let mX = sketch.mouseX + (canvasW / 2);
            if (mX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0) {
                x = Math.floor(mX / twoDtileSize);
                y = Math.floor(sketch.mouseY / twoDtileSize);
                if (!includesArray(screen.invalidRhomdod, [x, y, screen.layer]) && !threeScreen.removeRhomdod(x, y, layer)) {
                    threeScreen.addRhomdod(new RhomDod(x, y, layer, rgbColor));
                }
            }
        } else if (screen.shape === "cube") {
            let mX = sketch.mouseX + (canvasW / 2);
            if (mX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0) {
                x = Math.floor(mX / twoDtileSize);
                y = Math.floor(sketch.mouseY / twoDtileSize);
                if (!threeScreen.removeCube(x, y, layer)) {
                    threeScreen.addCube(new Cube(x, y, layer, rgbColor));
                }
            }
        } else if (screen.shape === "hexagon") {
            let mX = sketch.mouseX + (canvasW / 2);
            if (mX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0) {
                x = Math.floor(mX / twoDtileSize);
                y = Math.floor(sketch.mouseY / twoDtileSize);
                if (!threeScreen.removeHexagon(x, y, layer)) {
                    threeScreen.addHexagon(new Hexagon(x, y, layer, rgbColor));
                }
            }
        }
    }
}

function arraysEqual(arr1, arr2) {
    if (arr1.length !== arr2.length) return false;
    for (let i = 0; i < arr1.length; i++) {
        if (arr1[i] !== arr2[i]) return false;
    }
    return true;
}

function includesArray(mainArray, subArray) {
    for (let arr of mainArray) {
        if (arraysEqual(arr, subArray)) return true;
    }
    return false;
}

function areAdjacent(block1, block2) {
    // Assuming blocks are adjacent if they share a face
    return Math.abs(block1.x - block2.x) + Math.abs(block1.y - block2.y) + Math.abs(block1.z - block2.z) === 1;
}

function areAllCubesConnected(blocks) {
    if (blocks.length === 0) return true;

    // Create an adjacency list
    let adjacencyList = new Map();
    blocks.forEach(block => {
        adjacencyList.set(block, []);
    });

    // Populate the adjacency list
    blocks.forEach(block => {
        blocks.forEach(otherBlock => {
            if (block !== otherBlock && areAdjacent(block, otherBlock)) {
                adjacencyList.get(block).push(otherBlock);
            }
        });
    });

    // Perform DFS
    let visited = new Set();
    function dfs(block) {
        visited.add(block);
        adjacencyList.get(block).forEach(neighbor => {
            if (!visited.has(neighbor)) {
                dfs(neighbor);
            }
        });
    }

    // Start DFS from the first block
    dfs(blocks[0]);
    return visited.size === blocks.length;
}

function hexRound(q, r) {
    let x = q;
    let z = r;
    let y = -x - z;

    let rx = Math.round(x);
    let ry = Math.round(y);
    let rz = Math.round(z);

    let x_diff = Math.abs(rx - x);
    let y_diff = Math.abs(ry - y);
    let z_diff = Math.abs(rz - z);

    if (x_diff > y_diff && x_diff > z_diff) {
        rx = -ry - rz;
    } else if (y_diff > z_diff) {
        ry = -rx - rz;
    } else {
        rz = -rx - ry;
    }

    return { q: rx, r: rz };
}

function pixelToHex(x, y, size) {
    const vertDist = (Math.sqrt(3) * size);
    const horizDist = (1.5 * size);
    let q = (Math.floor(x / horizDist) * horizDist);
    let r = (Math.floor(y / vertDist) * vertDist);
    let off = (Math.floor(x / horizDist) - 1) * Math.floor(y / vertDist) + Math.floor(x / horizDist + Math.floor(y / vertDist));
    if (off&1) {
        r += vertDist / 2;
    }
    let values = hexRound(q, r);
    q = values.q;
    r = values.r;
    return [q, r];
}

twodCanv = new p5(sketch1);
threeDCanv = new p5(sketch2);
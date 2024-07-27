document.oncontextmenu = () => { return false; }
canvasW = window.screen.width * .99;
canvasH = window.screen.height * .8;
canvasZ = 75;
twoDtileSize = canvasW / (2 * 30);
layer = 0;
highlight = false;
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
    for (var i=0; i < blocks.length; i++){
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
    jsonOutput += "    \"order\": 3,\n"
    jsonOutput += "    \"axisSize\": " + maxSize + ",\n"
    jsonOutput += "    \"modules\": ["
    for (var i=0; i < blocks.length; i++){
	current_block = blocks[i];
	jsonOutput += "\n{\n	\"position\": [" + current_block.x + ", " + current_block.y + ", " + current_block.z + "],\n"
	jsonOutput += "	\"static\": true\n},"
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
        blocks = screen.getCubes;
        if (undoPressed) {
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
                document.getElementById("switch-shape").innerText = "Switch to Cube";
                screen.setShape("hexagon");
            } else {
                document.getElementById("switch-shape").innerText = "Switch to Hexagon";
                screen.setShape("cube");
            }
            switchShapePressed = false;
            clearPressed = true;
        }
        let message = areAllCubesConnected(blocks) ? "Yes" : "No";
        document.getElementById("checkConnectivity").innerText = "Connected: " + message;    
    }

    sketch.mousePressed = function () {
        if (!(sketch.mouseX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0)) return;
        if (screen.shape === "cube") {
            x = Math.floor(sketch.mouseX / twoDtileSize);
            y = Math.floor(sketch.mouseY / twoDtileSize);
            console.log(x + ", " + y);
            if (!screen.removeCube(x, y, screen.layer)) {
                screen.addCube(new Cube(x, y, screen.layer, rgbColor));
                historyStack.push({ action: 'add', x: x, y: y, z: screen.layer, color: rgbColor });
            } else {
                historyStack.push({ action: 'remove', x: x, y: y, z: screen.layer, color: rgbColor });
            }
        } else {
            x = pixelToHex(sketch.mouseX, sketch.mouseY, twoDtileSize).q;
            y = pixelToHex(sketch.mouseX, sketch.mouseY, twoDtileSize).r;
            console.log(x + ", " + y);
            if (!screen.removeHexagon(x, y, screen.layer)) {
                screen.addHexagon(new Hexagon(x, y, screen.layer));
                // historyStack.push({ action: 'add', x: x, y: y, z: screen.layer, color: rgbColor });
            } else {
                // historyStack.push({ action: 'remove', x: x, y: y, z: screen.layer, color: rgbColor });
            }
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
        let mX = sketch.mouseX + (canvasW / 2);
        if (mX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0) {
            x = Math.floor(mX / twoDtileSize);
            y = Math.floor(sketch.mouseY / twoDtileSize);
            if (!threeScreen.removeCube(x, y, layer)) {
                threeScreen.addCube(new Cube(x, y, layer, rgbColor));
            }
        }
    }
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
    const q = (x * Math.sqrt(3)/3 - y / 3) / size;
    const r = y * 2/3 / size;
    return hexRound(q, r);
}

twodCanv = new p5(sketch1);
threeDCanv = new p5(sketch2);
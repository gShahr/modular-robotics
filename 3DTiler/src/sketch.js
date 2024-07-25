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
    var jsonOutput = JSON.stringify(blocks, null, 2);
    dwnldAsTxt("3Dtiles.json", jsonOutput);
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
                    screen.addCube(new Cube(lastMove.x, lastMove.y, lastMove.z));
                    threeScreen.addCube(new Cube(lastMove.x, lastMove.y, lastMove.z));
                }
            }
            undoPressed = false;
        }
        if (redoPressed) {
            if (redoStack.length > 0) {
                let lastMove = redoStack.pop();
                historyStack.push(lastMove);
                if (lastMove.action === 'add') {
                    screen.addCube(new Cube(lastMove.x, lastMove.y, lastMove.z));
                    threeScreen.addCube(new Cube(lastMove.x, lastMove.y, lastMove.z));
                } else {
                    screen.removeCube(lastMove.x, lastMove.y, lastMove.z);
                    threeScreen.removeCube(lastMove.x, lastMove.y, lastMove.z);
                }
            }
            redoPressed = false;
        }
    }

    sketch.mousePressed = function () {
        console.log(sketch.mouseY);
        if (sketch.mouseX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0) {
            console.log(sketch.mouseX);
            x = Math.floor(sketch.mouseX / twoDtileSize);
            y = Math.floor(sketch.mouseY / twoDtileSize);
            console.log(x + ", " + y);
            if (!screen.removeCube(x, y, screen.layer)) {
                screen.addCube(new Cube(x, y, screen.layer));
                historyStack.push({ action: 'add', x: x, y: y, z: screen.layer });
            } else {
                historyStack.push({ action: 'remove', x: x, y: y, z: screen.layer });
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
        console.log(sketch._renderer);
        console.log("Setup", Object.keys(sketch));
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
            console.log(x + ", " + y);
            if (!threeScreen.removeCube(x, y, layer)) {
                threeScreen.addCube(new Cube(x, y, layer, rgbColor));
            }
        }
    }
}

twodCanv = new p5(sketch1);
threeDCanv = new p5(sketch2);
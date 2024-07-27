document.oncontextmenu = () => {return false;}
canvasW = window.screen.width*.99;
canvasH = window.screen.height*.8;
canvasZ = 75;
twoDtileSize = canvasW/(2*30);
layer = 0;
highlight = false;
blocks = [];

function saveConfig() {
    var output = "";
    for(i = 0; i < blocks.length; i++){
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
    //var jsonOutput = JSON.stringify(blocks, null, 2);
    dwnldAsTxt("3Dtiles.scen", jsonOutput);
}

function exportToScen() {
    var ScenOutput = "";
    ScenOutput += "0 244 244 0 100\n\n"
    for (var i=0; i < blocks.length; i++){
	if(i < 10){
		ScenOutput += "0";
	}
	current_block = blocks[i];
	//console.log(current_block.x,current_block.y,current_block.z);
	//block_info = [i,current_block.x,current_block.y,current_block.z];
	ScenOutput += i + "," + "0" + "," + current_block.x + "," + current_block.y + "," + current_block.z + "\n";
    }
    //var jsonOutput = JSON.stringify(blocks, null, 2);
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

var sketch1 = function(sketch) {
    sketch.setup = function() {
        canv1 = sketch.createCanvas(canvasW / 2, canvasH);
        canv1.position(0,30);
        screen = new twoDScreen(canvasW / 2, canvasH, twoDtileSize);
        prevLayer = layer;
    }
    sketch.draw = function() {
        if(layer > prevLayer){
            screen.upLayer();
        }
        if(layer < prevLayer){
            screen.downLayer();
        }
        sketch.background(0, 255, 0);
        screen.draw(sketch);
        prevLayer = layer;
        blocks = screen.getCubes;
    }
    
    sketch.mousePressed = function() {
        console.log(sketch.mouseY);
        if (sketch.mouseX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0){
            console.log(sketch.mouseX);
            x = math.floor(sketch.mouseX/twoDtileSize);
            y = math.floor(sketch.mouseY/twoDtileSize);
            console.log(x + ", " + y);
            if (!screen.removeCube(x,y,screen.layer)) {
                screen.addCube(new Cube(x, y, screen.layer));
            }
            let cubeXs = screen.cubes.map(cube => cube.x);
            let cubeYs = screen.cubes.map(cube => cube.y);
            let cubeZs = screen.cubes.map(cube => cube.z);

            let xSum = cubeXs.reduce((sum, cur) => sum + cur, 0);
            let ySum = cubeYs.reduce((sum, cur) => sum + cur, 0);
            let zSum = cubeZs.reduce((sum, cur) => sum + cur, 0);

            let cX = xSum/cubeXs.length;
            let cY = ySum/cubeYs.length;
            let cZ = zSum/cubeZs.length;
        }
    }
}

var sketch2 = function(sketch) {
    sketch.setup = function() {
        canv2 = sketch.createCanvas(canvasW / 2, canvasH, sketch.WEBGL);
        canv2.position(canvasW / 2, 30);
        threeScreen = new threeDScreen(canvasW / 2, canvasH, twoDtileSize / 5);
        sketch._center = [0, 0, 0];
        console.log(sketch._renderer);
        console.log("Setup", Object.keys(sketch));
        sketch.createEasyCam();
    }

    sketch.draw = function() {
        sketch.background(205, 102, 94);
        threeScreen.draw(sketch, highlight, layer);
    }
    
    sketch.mousePressed = function() {
        let mX = sketch.mouseX+(canvasW/2);
        if (mX < canvasW / 2 && sketch.mouseY < canvasH && sketch.mouseY > 0){
            x = math.floor(mX/twoDtileSize);
            y = math.floor(sketch.mouseY/twoDtileSize);
            console.log(x + ", " + y);
            if (!threeScreen.removeCube(x, y ,layer)) {
                threeScreen.addCube(new Cube(x, y, layer));
            }
        }
    }
}

twodCanv = new p5(sketch1);
threeDCanv = new p5(sketch2);
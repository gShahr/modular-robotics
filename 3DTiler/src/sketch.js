// import "convexHull.js"
document.oncontextmenu = () => {return false;}
canvasW = window.screen.width*.99;
canvasH = window.screen.height*.8;
canvasZ = 75;
twoDtileSize = canvasW/(2*30);
layer = 0;
highlight = false;

function saveConfig(){
    var output = "";
    for(i = 0; i < blocks.length; i++){
        output += blocks[i].x + "," + blocks[i].y + ",";
    }

    dwnldAsTxt("3Dtiles.txt", output);

}

function dwnldAsTxt(filename, text){
    var element = document.createElement('a');
    element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
    element.setAttribute('download', filename);
  
    element.style.display = 'none';
    document.body.appendChild(element);
  
    element.click();
  
    document.body.removeChild(element);
}

function ChangeLabel(newString){
    document.getElementById("curLayer").textContent = newString;
}

function upLayer(){
    layer++;
    ChangeLabel("Current Layer = " + layer);
}

function downLayer(){
    layer--;
    ChangeLabel("Current Layer = " + layer);
}

function highlightLayer(){
    highlight = !highlight;
}

var sketch1 = function(sketch){
    
    sketch.setup = function(){
        canv1 = sketch.createCanvas(canvasW/2, canvasH);
        canv1.position(0,30);
        screen = new twoDScreen(canvasW/2, canvasH, twoDtileSize);
        prevLayer = layer;
    }
    
    sketch.draw = function(){
        if(layer > prevLayer){
            screen.upLayer();
        }
        if(layer < prevLayer){
            screen.downLayer();
        }
        sketch.background(0, 255, 0);
        screen.draw(sketch);
        prevLayer = layer;
        // sketch.rect(10, 10, 1000, 1000); 
    }
    
    sketch.mousePressed = function(){
        console.log(sketch.mouseY);
        if(sketch.mouseX < canvasW/2 && sketch.mouseY < canvasH && sketch.mouseY > 0){
            console.log(sketch.mouseX);
            x = math.floor(sketch.mouseX/twoDtileSize);
            y = math.floor(sketch.mouseY/twoDtileSize);
            console.log(x + ", " + y);

            if(!screen.removeCube(x,y,screen.layer)){
                screen.addCube(new Cube(x,y,screen.layer));
            }


            //get new center

            let cubeXs = screen.cubes.map(cube => cube.x);
            let cubeYs = screen.cubes.map(cube => cube.y);
            let cubeZs = screen.cubes.map(cube => cube.z);


            // console.log(cubeXs);
            // cX = (Math.max(...cubeXs) - Math.min(...cubeXs))/2 +
            //         Math.min(...cubeXs);


            // cY = (Math.max(...cubeYs) - Math.min(...cubeYs))/2 +
            //         Math.min(...cubeYs);     
                    
            // cZ = (Math.max(...cubeZs) - Math.min(...cubeZs))/2 +
            //      Math.min(...cubeZs);

            let xSum = cubeXs.reduce((sum, cur) => sum + cur, 0);
            let ySum = cubeYs.reduce((sum, cur) => sum + cur, 0);
            let zSum = cubeZs.reduce((sum, cur) => sum + cur, 0);

            let cX = xSum/cubeXs.length;
            let cY = ySum/cubeYs.length;
            let cZ = zSum/cubeZs.length;

            if(0 === 1){

                threeDCanv.newCenter(cX, cY, cZ);
                set3DCenter();
            }
        }
    }
}

new p5(sketch1);

var sketch2 = function(sketch){
    sketch.setup = function() {


        canv2 = sketch.createCanvas(canvasW/2, canvasH, sketch.WEBGL);
        canv2.position(canvasW/2, 30);
        threeScreen = new threeDScreen(canvasW/2, canvasH, twoDtileSize/5);
        sketch._center = [0,0,0];
        console.log(sketch._renderer);
        console.log("Setup", Object.keys(sketch));

        sketch.createEasyCam();

    }


    sketch.draw = function(){
        // sketch.orbitControl();
        sketch.background(205, 102, 94);

        threeScreen.draw(sketch, highlight, layer);

    }
    
    sketch.mousePressed = function(){
        let mX = sketch.mouseX+(canvasW/2);
        if(mX < canvasW/2 && sketch.mouseY < canvasH && sketch.mouseY > 0){
            x = math.floor(mX/twoDtileSize);
            y = math.floor(sketch.mouseY/twoDtileSize);
            console.log(x + ", " + y);

            if(!threeScreen.removeCube(x,y,layer)){
                threeScreen.addCube(new Cube(x,y,layer));
            }
        }


    }

}


threeDCanv = new p5(sketch2);



//These are obsolete with the easy cam library
//Keeping them around for posterity though
threeDCanv.newCenter = (x,y,z) => {
    threeDCanv.center = [x,y,z];
    console.log("new 3d center", threeDCanv.center);
};

function set3DCenter(){


    // console.log(threeDCanv.center);
    threeDCanv.setCameraVal = () => {
        threeDCanv.camera(
            0,0, (canvasH/2)/(Math.tan(Math.PI/6)), //eye position default value
            threeDCanv.center[0]*twoDtileSize,
            threeDCanv.center[1]*twoDtileSize,
            threeDCanv.center[2]*twoDtileSize, //center position
            0, 1, 0 //portion of axis in "up" direction
             );
    }
    threeDCanv.setCameraVal();

}



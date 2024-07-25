class threeDScreen {
    constructor(width, height, tileSize) {
        this.width    = width
        this.height   = height;
        this.tileSize = tileSize*5;
        this.cubes    = [];
    }

    addCube(cube) {
        this.cubes.push(cube);
    }

    removeCube(x,y,z) {
        for(let i = 0; i < this.cubes.length; i++){
            if(this.cubes[i].x === x 
                && this.cubes[i].y === y
                && this.cubes[i].z === z){
                    this.cubes.splice(i,1);
                    console.log("here");
                    console.log(this.cubes.length);
                    return true;
                }
        }
        return false;
    }

    draw(sketch, highlight, highLayer) {
        sketch.fill(255);
        sketch.translate(-this.width/2, -this.height/2);
        for (let i = 0; i < this.cubes.length; i++){
            sketch.translate(this.cubes[i].x*this.tileSize,
                                 this.cubes[i].y*this.tileSize, 
                                 this.cubes[i].z*this.tileSize);
            if (highlight && this.cubes[i].z === highLayer) {
                sketch.fill(0, 0, 255);
            }
            sketch.box(this.tileSize);
            sketch.fill(255);
            sketch.translate(-this.cubes[i].x*this.tileSize,
                                 -this.cubes[i].y*this.tileSize,
                                 -this.cubes[i].z*this.tileSize);
        }
        sketch.translate(this.width/2, this.height/2);
    }
}
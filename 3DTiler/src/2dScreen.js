class twoDScreen{


    constructor(width, height, tileSize){
        this.width    = width
        this.height   = height;
        this.tileSize = tileSize;
        this.layer    = 0;
        this.cubes    = [];
    }

    set layer(value){
        this._layer = value;
    }
    
    get layer(){
        return this._layer;
    }

    get getCubes(){
        return this.cubes;
    }


    removeCube(x,y,z){
            for(let i = 0; i < this.cubes.length; i++){
                if(this.cubes[i].x === x 
                    && this.cubes[i].y === y
                    && this.cubes[i].z === z){
                        this.cubes.splice(i,1);
                        return true;
                    }
            }
            return false;
    }

    draw(sketch){
        sketch.background(255);
        sketch.stroke(0);
        for(let i = 0; i < this.width; i++){
            sketch.line(0, i*this.tileSize, this.width, i*this.tileSize);
            sketch.line(i*this.tileSize, 0, i*this.tileSize, this.height);
        }

        sketch.stroke(255);
        for(let i = 0; i < this.cubes.length; i++){
            
            switch(this.cubes[i].z){
                case this.layer:
                    sketch.fill(0);
                    sketch.rect(
                        this.cubes[i].x*this.tileSize, 
                        this.cubes[i].y*this.tileSize, 
                        this.tileSize, 
                        this.tileSize);
                    sketch.fill(255);
                    break;
                
                case this.layer-1:
                    sketch.fill(166, 166, 166);
                    sketch.rect(
                        this.cubes[i].x*this.tileSize, 
                        this.cubes[i].y*this.tileSize, 
                        this.tileSize, 
                        this.tileSize);
                    sketch.fill(255);
            }
            // if(this.cubes[i].z === this.layer){
                
            // }
        }
    }

    upLayer(){
        this.layer++;
    }
    downLayer(){
        this.layer--;
    }

    addCube(cube){
        this.cubes.push(cube);
    }
}
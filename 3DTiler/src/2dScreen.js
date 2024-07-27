class twoDScreen {
    constructor(width, height, tileSize) {
        this.width    = width
        this.height   = height;
        this.tileSize = tileSize;
        this.layer    = 0;
        this.cubes    = [];
        this.hexagons = [];
        this.shape = "cube";
    }

    set layer(value) {
        this._layer = value;
    }
    
    get layer() {
        return this._layer;
    }

    get getCubes() {
        return this.cubes;
    }

    set setCubes(cubes) {
        this._cubes = cubes;
    }

    get getHexagons() {
        return this.hexagons;
    }

    set setHexagons(hexagons) {
        this._hexagons = hexagons;
    }

    get getShape() {
        return this.shape;
    }

    set setShape(shape) {
        this._shape = shape;
    }

    upLayer() {
        this.layer++;
    }

    downLayer() {
        this.layer--;
    }

    addCube(cube){
        this.cubes.push(cube);
    }

    addHexagon(hexagon) {
        this.hexagons.push(hexagon);
    }

    setShape(shape) {
        this.shape = shape;
    }

    removeCube(x, y, z) {
        for (let i = 0; i < this.cubes.length; i++) {
            if (this.cubes[i].x === x && this.cubes[i].y === y && this.cubes[i].z === z) {
                this.cubes.splice(i, 1);
                return true;
            }
        }
        return false;
    }

    removeHexagon(x, y, z) {
        for (let i = 0; i < this.hexagons.length; i++) {
            if (this.hexagons[i].x === x && this.hexagons[i].y === y && this.hexagons[i].z === z) {
                this.hexagons.splice(i, 1);
                return true;
            }
        }
        return false;
    }

    removeAllCubes() {
        while (this.cubes.length > 0) {
            this.removeCube(this.cubes.pop());
        }
    }

    hexagon = (sketch, centerX, centerY, radius) => {
        const angle = Math.PI / 3;
        sketch.beginShape();
        for (let i = 0; i < 6; i++) {
            const x_i = centerX + radius * Math.cos(angle * i);
            const y_i = centerY + radius * Math.sin(angle * i);
            sketch.vertex(x_i, y_i);
        }
        sketch.endShape(sketch.CLOSE);
    };

    drawHexGrid = (sketch, width, height, tileSize) => {
        const vertDist = Math.sqrt(3) * tileSize;
        const horizDist = 1.5 * tileSize;
        let count = 0;

        for (let y = 0; y < height; y += vertDist) {
            for (let x = 0; x < width; x += horizDist) {
                const yOffset = count % 2 === 0 ? 0 : vertDist / 2;
                this.hexagon(sketch, x, y + yOffset, tileSize);
                count++;
            }
            count++;
        }
    };

    drawCubes(sketch) {
        for(let i = 0; i < this.width; i++) {
            sketch.line(0, i*this.tileSize, this.width, i*this.tileSize);
            sketch.line(i*this.tileSize, 0, i*this.tileSize, this.height);
        }
        for (let i = 0; i < this.cubes.length; i++) {
            switch(this.cubes[i].z) {
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
        }
    }

    drawHexagons(sketch) {
        this.drawHexGrid(sketch, this.width, this.height, this.tileSize);
        for (let i = 0; i < this.hexagons.length; i++) {
            switch(this.hexagons[i].z) {
                case this.layer:
                    sketch.fill(0);
                    this.hexagon(sketch, this.hexagons[i].x, this.hexagons[i].y, this.tileSize);
                    sketch.fill(255);
                    break;
                case this.layer-1:
                    sketch.fill(166, 166, 166);
                    this.hexagon(sketch, this.hexagons[i].x, this.hexagons[i].y, this.tileSize);
                    sketch.fill(255);
            }
        }
    }

    draw(sketch) {
        sketch.background(255);
        sketch.stroke(0);
        if (this.shape == "cube") {
            this.drawCubes(sketch);
        } else if (this.shape == "hexagon") {
            this.drawHexagons(sketch);
        }
        sketch.stroke(255);
    }
}
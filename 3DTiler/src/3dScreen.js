class threeDScreen {
    constructor(width, height, tileSize) {
        this.width    = width
        this.height   = height;
        this.tileSize = tileSize*5;
        this.cubes    = [];
        this.hexagons = [];
        this.shape    = "cube";
    }

    get getCubes() {
        return this.cubes;
    }

    set setCubes(cubes) {
        this._cubes = cubes;
    }

    addCube(cube) {
        this.cubes.push(cube);
    }

    addHexagon(hexagon) {
        this.hexagons.push(hexagon);
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
        return false
    }

    removeAllCubes() {
        while (this.cubes.length > 0) {
            this.removeCube(this.cubes.pop());
        }
    }

    removeAllHexagons() {
        while (this.hexagons.length > 0) {
            this.removeHexagon(this.hexagons.pop());
        }
    }

    drawCubes(sketch) {
        const halfWidth = this.width / 2;
        const halfHeight = this.height / 2;
    
        for (let i = 0; i < this.cubes.length; i++) {
            const x = this.cubes[i].x * this.tileSize - halfWidth;
            const y = this.cubes[i].y * this.tileSize - halfHeight;
            const z = this.cubes[i].z * this.tileSize;
            sketch.push();
            sketch.translate(x, y, z);
            if (highlight && this.cubes[i].z === highLayer) {
                sketch.fill(0, 0, 255);
            } else {
                sketch.fill(this.cubes[i].color[0], this.cubes[i].color[1], this.cubes[i].color[2]);
            }
            sketch.box(this.tileSize);
            sketch.pop();
        }
    }

    drawHexagons(sketch) {
        const halfWidth = this.width / 2;
        const halfHeight = this.height / 2;
        const depth = this.tileSize;
    
        for (let i = 0; i < this.hexagons.length; i++) {
            console.log(this.hexagons[0]);
            const x = this.hexagons[i].x * this.tileSize - halfWidth;
            const y = this.hexagons[i].y * this.tileSize - halfHeight;
            const z = this.hexagons[i].z * this.tileSize;
            sketch.push();
            sketch.translate(x, y, z);
    
            if (highlight && this.hexagons[i].z === highLayer) {
                sketch.fill(0, 0, 255);
            } else {
                sketch.fill(this.hexagons[i].color[0], this.hexagons[i].color[1], this.hexagons[i].color[2]);
            }
    
            // Draw top face
            sketch.beginShape();
            for (let j = 0; j < 6; j++) {
                const angle = sketch.TWO_PI / 6 * j;
                const hx = this.tileSize * Math.cos(angle);
                const hy = this.tileSize * Math.sin(angle);
                sketch.vertex(hx, hy, 0);
            }
            sketch.endShape(sketch.CLOSE);
    
            // Draw bottom face
            sketch.beginShape();
            for (let j = 0; j < 6; j++) {
                const angle = sketch.TWO_PI / 6 * j;
                const hx = this.tileSize * Math.cos(angle);
                const hy = this.tileSize * Math.sin(angle);
                sketch.vertex(hx, hy, depth);
            }
            sketch.endShape(sketch.CLOSE);
    
            // Draw sides
            for (let j = 0; j < 6; j++) {
                const angle1 = sketch.TWO_PI / 6 * j;
                const angle2 = sketch.TWO_PI / 6 * ((j + 1) % 6);
                const hx1 = this.tileSize * Math.cos(angle1);
                const hy1 = this.tileSize * Math.sin(angle1);
                const hx2 = this.tileSize * Math.cos(angle2);
                const hy2 = this.tileSize * Math.sin(angle2);
    
                sketch.beginShape();
                sketch.vertex(hx1, hy1, 0);
                sketch.vertex(hx2, hy2, 0);
                sketch.vertex(hx2, hy2, depth);
                sketch.vertex(hx1, hy1, depth);
                sketch.endShape(sketch.CLOSE);
            }
    
            sketch.pop();
        }
    }
    
    draw(sketch) {
        sketch.fill(255);
        this.drawHexagons(sketch);
        return;
        if (this.shape == "cube") {
            this.drawCubes(sketch);
        } else if (this.shape == "hexagon") {
            this.drawHexagons(sketch);
        }
    }
}
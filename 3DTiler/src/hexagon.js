class Hexagon {
    constructor(x, y, z, static = false, color = [255, 255, 255]) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.static = static;
        this.color = color;
    }

    set x(val) {
        this._x = val;
    }

    get x() {
        return this._x;
    }

    set y(val) {
        this._y = val;
    }

    get y() {
        return this._y;
    }

    set z(val) {
        this._z = val;
    }

    get z() {
        return this._z;
    }

    set color(val) {
        this._color = val;
    }

    get color() {
        return this._color;
    }
}
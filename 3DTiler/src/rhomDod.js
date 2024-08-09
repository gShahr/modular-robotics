class RhomDod {
    constructor(x, y, z, color = [255, 255, 255], mStatic = false) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.mStatic = mStatic;
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
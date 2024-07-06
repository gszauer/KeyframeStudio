export default class UIToolBarShelf {
    _scene = null;
    _visible = true;
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    /* Override these (and call them) */
    constructor(scene) {
        this._scene = scene;
    }

    SetVisibility(value) {
        this._visible = value;
    }

    UpdateColors() {}

    Layout(x, y, width, height) {
        if (x === undefined) { x = this._x; }
        if (y === undefined) { y = this._y; }
        if (width === undefined) { width = this._width; }
        if (height === undefined) { height = this._height; }

        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;
    }
    /* End overrides */

    Hide() {
        if (this._visible) {
            this.SetVisibility(false);
            this._visible = false;
        }
    }

    Show() {
        if (!this._visible) {
            this.SetVisibility(true);
            this._visible = true;
        }
    }
}
import UIGlobals from './UIGlobals.js'

export default class UIView {
    _scene = null;
    _parent = null;

    // Only set in layout
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    constructor(scene, parent = null) {
        this._scene = scene;
        this._parent = parent;
    }

    UpdateColors() {

    }

    Layout(x, y, width, height) {
        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;
    }
}
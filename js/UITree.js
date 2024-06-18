import UIGlobals from './UIGlobals.js'
import UIScrollView from './UIScrollView.js'
import UITreeNode from './UITreeNode.js'

export default class UITree {
    _scene = null;
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    _scrollView = null;
    _inputItem = null;

    constructor(scene) {
        this._scene = scene;

        this._scrollView = new UIScrollView(scene, this);
    
        this._inputItem = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._inputItem.setDepth(UIGlobals.WidgetLayer - 2);
        this._inputItem.setOrigin(0, 0);
        this._inputItem.setInteractive();
        scene.input.setDraggable(this._inputItem);
    }

    UpdateColors() {
        this._scrollView.UpdateColors();
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._inputItem.setPosition(x, y);
        this._inputItem.setScale(width, height);

        this._scrollView.Layout(x, y, width, height);
    }

    SetVisibility(value) {
        this._scrollView.SetVisibility(value);
        this._inputItem.setActive(value).setVisible(value);
    }
}
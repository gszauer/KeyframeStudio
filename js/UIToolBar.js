import UIGlobals from './UIGlobals.js'

export default class UIToolBar {
    _scene = null;

    _shelves = null;
    _active = null;

    _y = 0;
    _width = 0;
    _height = 0;
    
    _backgroundSprite = null;
    _seperatorSprite = null;

    constructor(scene) {
        this._scene = scene;
        const self = this;

        self._seperatorSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._seperatorSprite.setDepth(UIGlobals.WidgetLayer);
        self._seperatorSprite.setOrigin(0, 0);

        self._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        self._backgroundSprite.setOrigin(0, 0);

        this._shelves = new Map();
    }
   
    Layout() {
        const self = this;
       
        self._width = self._scene.scale.width;
        self._height = UIGlobals.Sizes.EditorBarHeight;
        self._x = UIGlobals.Sizes.ToolboxWidth;
        self._y = 0;

        self._seperatorSprite.setTint(UIGlobals.Colors.BorderDecorative);
        self._seperatorSprite.setPosition(self._x, self._y);
        self._seperatorSprite.setScale(self._width, self._height);

        const half = UIGlobals.Sizes.EditorBarSeperatorHeight;
        self._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
        self._backgroundSprite.setPosition(self._x, self._y + half);
        self._backgroundSprite.setScale(self._width, self._height - half * 2);

        if (this._active != null) {
            this._active.Layout(self._x, 0, self._width, self._height);
        }
    }

    AddShelf(name, shelf) {
        this._shelves.set(name, shelf);
        shelf.Hide();
    }

    Activate(name = null) {
        if (this._active != null) {
            this._active.Hide();
            this._active = null;
        }

        if (name != null) {
            this._active = this._shelves.get(name);
        }

        if (this._active != null) {
            this._active.Show();
            this._active.Layout(0, 0, this._width, this._height);
        }
    }
}
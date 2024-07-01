import UIGlobals from './UIGlobals.js'

export default class UIToolBar {
    _scene = null;

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
    }
   
    Layout() {
        const self = this;
       
        self._width = self._scene.scale.width;
        self._height = UIGlobals.Sizes.EditorBarHeight;
        self._y = 0;

        self._seperatorSprite.setTint(UIGlobals.Colors.BorderDecorative);
        self._seperatorSprite.setPosition(0, self._y);
        self._seperatorSprite.setScale(self._width, self._height);

        const half = UIGlobals.Sizes.EditorBarSeperatorHeight;
        self._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
        self._backgroundSprite.setPosition(0, self._y + half);
        self._backgroundSprite.setScale(self._width, self._height - half * 2);

        const left = 0;
        const top = self._y + half;
        const right = left + self._width;
        const bottom = self._y + self._height - half * 2;
        // TODO: Layout children in the above rectangle
    }

    Destroy() {
        // TODO
    }
}
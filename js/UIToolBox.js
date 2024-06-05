import UIGlobals from './UIGlobals.js'

export default class UIToolBox {
    _scene = null;

    _y = 0;
    _width = 0;
    _height = 0;
    
    _backgroundSprite = null;
    _borderSprite = null;

    constructor(scene) {
        this._scene = scene;

        this._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._borderSprite.setDepth(UIGlobals.WidgetLayer);
        this._borderSprite.setOrigin(0, 0);

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);
    }

    Layout() {
        let x = 0;
        let y = this._y = UIGlobals.Sizes.TopMenuHeight + UIGlobals.Sizes.EditorBarHeight;
        let width = this._width = UIGlobals.Sizes.ToolboxWidth;
        let height = this._height = this._scene.scale.height - y;

        this._borderSprite.setPosition(x, y);
        this._borderSprite.setScale(width, height);
        this._borderSprite.setTint(UIGlobals.Colors.BorderDecorative);

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(width - UIGlobals.Sizes.ToolboxPadding, height);
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);

    }
}
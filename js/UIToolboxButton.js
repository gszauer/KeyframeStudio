import UIGlobals from './UIGlobals.js'

export default class UIToolBoxButton {
    _scene = null;
    _callback = null;

    _backgroundSprite = null;
    _iconSprite = null;

    constructor(scene, iconName = "", callback = null) {
        this._scene = scene;
        this._callback = callback;

        const self = this;

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0.5, 0.5);

        this._iconSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, iconName);
        this._iconSprite.setDepth(UIGlobals.WidgetLayer);
        const iconScaleX =  UIGlobals.Sizes.ToolboxButtonSize  /  this._iconSprite.width ;
        const iconScaleY =   UIGlobals.Sizes.ToolboxButtonSize /  this._iconSprite.height;
        this._iconSprite.setScale(iconScaleX, iconScaleY);
    }

    UpdateColors() {
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
        this._iconSprite.setTint(UIGlobals.Colors.Icon);
    }

    Layout(x, y) {
       const width = UIGlobals.Sizes.ToolboxButtonSize;
       const height = UIGlobals.Sizes.ToolboxButtonSize;

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(width, height);

        this._iconSprite.setPosition(x, y);

        this.UpdateColors();
    }
    
}
import UIGlobals from './UIGlobals.js'

export default class UIStringBox {
    _scene = null;
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;
    _defaultWidth = 150;

    _text = ""; // always a string
    _bitmapText = null; // phaser bitmap text object
    _borderSprite = null;
    _backgroundSprite = null;

    _disabled = false;
    _maskRect = null;

    get text() {
        if (this._text == null) {
            return "";
        }
        return this._text;
    }

    set text(value) {
        this._text = value;
        this._bitmapText.text = value;
    }

    get number() {
        if (this._text == null) {
            return 0;
        }
        let numString = "";
        const txt = this._text;
        for (let i = 0; i < txt.length; ++i) {
            if (txt[i] >= '0' && txt[i] <= '9') {
                numString += txt[i];
            }
        }
        if (numString == "") {
            numString = "0";
        }

        return Number(numString);
    }

    set number(value) {
        this.text = "" + value;
    }

    constructor(scene, text = "", defaultWidth = 0) {
        this._scene = scene;
        const self = this;
        self._defaultWidth = defaultWidth;
        if (defaultWidth == 0) { self._defaultWidth = 150; }
        self._text = text;

        self._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._borderSprite.setDepth(UIGlobals.WidgetLayer);
        self._borderSprite.setOrigin(0, 0);

        self._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        self._backgroundSprite.setOrigin(0, 0);

        self._bitmapText = scene.add.bitmapText(0, 0, UIGlobals.Font100, name);
        self._bitmapText.setDepth(UIGlobals.WidgetLayer);
        self._bitmapText.text = text;

        self._maskRect = scene.add.rectangle(0, 0, 100, 100, 0x000000).setVisible(false).setOrigin(0, 0);
        const mask = self._maskRect.createGeometryMask();
        self._bitmapText.setMask(mask);
    }

    AddToContainer(container) {
        container.add(this._borderSprite);
        container.add(this._backgroundSprite);
        container.add(this._bitmapText);
        container.add(this._maskRect);
    }

    UpdateColors() {
        let borderTint = UIGlobals.Colors.ElementBorderTintIdle;
        let backgroundTint = UIGlobals.Colors.BackgroundLayer1;

        if (UIGlobals.Hot == this._bitmapText) {
            borderTint = UIGlobals.Colors.ElementBorderTintHot;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }
        if (UIGlobals.Active == this._bitmapText) {
            borderTint = UIGlobals.Colors.ElementBorderTintActive;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }
        if (this._disabled) {
            backgroundTint = UIGlobals.Colors.BackgroundLayer1;
            borderTint = UIGlobals.Colors.IconDisabled;
        }

        this._backgroundSprite.setTint(backgroundTint);
        this._borderSprite.setTint(borderTint);
        this._bitmapText.setTint(borderTint);
    }
    
    Disable() {
        this._disabled = true;
        this.UpdateColors();
    }

    Enable() {
        this._disabled = false;
        this.UpdateColors();
    }
    
    Layout(x, y, width = 0, height_ /* doesn't matter*/ = 0) {
        const self = this;
        
        self._x = x;
        self._y = y;
        self._width = width;
        if (width == 0) {
            self._width = self._defaultWidth;
            width = self._defaultWidth;
        }
        const height = self._height = UIGlobals.Sizes.TextboxHeight;
        const border = UIGlobals.Sizes.TextBoxBorderSize;
        const margin = UIGlobals.Sizes.TextboxTextMargin;

        self._borderSprite.setPosition(x, y);
        self._borderSprite.setScale(width, height);

        self._backgroundSprite.setPosition(x + border, y + border);
        self._backgroundSprite.setScale(width - border * 2, height - border * 2);

        self._bitmapText.setPosition(x + border + margin, y + border);


        self._maskRect.setPosition(x, y);
        self._maskRect.setSize(width - border - margin, height)
        
        self.UpdateColors();
    }

    SetVisibility(visible) {
        this._bitmapText.setActive(visible).setVisible(visible);
        this._borderSprite.setActive(visible).setVisible(visible);
        this._backgroundSprite.setActive(visible).setVisible(visible);
    }

    Show() {
        this.SetVisibility(true);
    }

    Hide() {
        this.SetVisibility(false);
    }
}
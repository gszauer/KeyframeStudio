import UIGlobals from './UIGlobals.js'

export default class UITextButton {
    _scene = null;
    _text = null;
    onClick = null;

    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    _borderSprite = null;
    _backgroundSprite = null;
    _textLabel = null;

    constructor(scene, buttonText="", _onClick = null) {
        this._scene = scene;
        this._text = buttonText;
        this.onClick = _onClick;

        this._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._borderSprite.setOrigin(0, 0);
        this._borderSprite.setDepth(UIGlobals.WidgetLayer);

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setOrigin(0, 0);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);

        this._textLabel = scene.add.bitmapText(0, 0, UIGlobals.Font100, "");
        this._textLabel.setDepth(UIGlobals.WidgetLayer);
        this._textLabel.text = buttonText;

        const borderSprite = this._borderSprite;
        const self = this;
        borderSprite.setInteractive();

        borderSprite.on("pointerover", function (pointer, localX, localY, event) {
            if (UIGlobals.Active == null) {
                UIGlobals.Hot = borderSprite;
            }

            self.UpdateColors();
        });
        borderSprite.on("pointerout", function (pointer, event) {
            if (UIGlobals.Hot == borderSprite) {
                UIGlobals.Hot = null;
            }

            self.UpdateColors();
        });
        borderSprite.on("pointerdown", function (pointer, localX, localY, event) {
            UIGlobals.Active = borderSprite;

            self.UpdateColors();
        });

        scene.input.on("pointerup", function(pointer, currentlyOver) {
            if (UIGlobals.Active != null && UIGlobals.Active == borderSprite) {
                let left = borderSprite.x;
                let right = left + borderSprite.scaleX;
                let top = borderSprite.y;
                let bottom = top + borderSprite.scaleY;

                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    if (self.onClick != null) {
                        self.onClick(self);
                    }
                }
                
                UIGlobals.Active = null;
                self.UpdateColors();
            }
        });
    }

    UpdateColors() {
        let borderTint = UIGlobals.Colors.ElementBorderTintIdle;
        let backgroundTint = UIGlobals.Colors.BackgroundLayer1;

        if (UIGlobals.Hot == this._borderSprite) {
            borderTint = UIGlobals.Colors.ElementBorderTintHot;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }
        if (UIGlobals.Active == this._borderSprite) {
            borderTint = UIGlobals.Colors.ElementBorderTintActive;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }

        this._borderSprite.setTint(borderTint);
        this._backgroundSprite.setTint(backgroundTint);
        this._textLabel.setTint(borderTint);
    }

    Layout(x, y, w, h) {
        if (w < 0) { w = 0; }
        if (h < 0) { h = 0; }

        this._x = x;
        this._y = y;
        this._width = w;
        this._height = h;

        this._borderSprite.setPosition(x, y);
        this._borderSprite.setScale(w, h);

        x += 2; y += 2;
        w -= 4; h -= 4;

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(w, h);

        const textWidth = this._textLabel.width;
        x = x + w / 2 - textWidth / 2;
        
        this._textLabel.setPosition(x, y - 3);

        
    }

    SetVisibility(value) {
        this._borderSprite.setActive(value).setVisible(value);
        this._backgroundSprite.setActive(value).setVisible(value);
        this._textLabel.setActive(value).setVisible(value);
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
import UIGlobals from './UIGlobals.js'

export default class UIToggle {
    _scene = null;
    _state = false;
    _x = 0;
    _y = 0;
    _text = null;
    _border = null;
    _background = null;
    _check1 = null;
    _check2 = null;
    _onToggle = null; // arguments: bool value, UIToggle this

    constructor(scene, text="", onToggle = null) {
        this._scene = scene;
        const self = this;

        self._onToggle = onToggle;

        self._border = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._border.setOrigin(0, 0);
        self._border.setDepth(UIGlobals.WidgetLayer);
        const borderSprite = self._border;

        self._background = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._background.setOrigin(0, 0);
        self._background.setDepth(UIGlobals.WidgetLayer);

        self._check1 = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._check1.setOrigin(0.5, 0.5);
        self._check1.setDepth(UIGlobals.WidgetLayer);

        self._check2 = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._check2.setOrigin(0.5, 0.5);
        self._check2.setDepth(UIGlobals.WidgetLayer);

        self._text = scene.add.bitmapText(0, 0, UIGlobals.Font100, name);
        self._text.setDepth(UIGlobals.WidgetLayer);
        self._text.text = text;

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
                    self._state = !self._state;
                    console.log("Toggle " + self._text.text + " to " + self._state);
                    if (self._onToggle != null) {
                        self._onToggle(self._state, self);
                    }
                }
                
                UIGlobals.Active = null;
                self.UpdateColors();
            }
        });
    }

    UpdateColors() {
        const self = this;

        let borderTint = UIGlobals.Colors.ElementBorderTintIdle;
        let backgroundTint = UIGlobals.Colors.BackgroundLayer1;

        if (UIGlobals.Hot == this._border) {
            borderTint = UIGlobals.Colors.ElementBorderTintHot;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }
        if (UIGlobals.Active == this._border) {
            borderTint = UIGlobals.Colors.ElementBorderTintActive;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }
        
        self._text.setTint(borderTint);

        
        self._border.setTint(borderTint);
        self._background.setTint(backgroundTint);
        self._check1.setTint(borderTint);
        self._check2.setTint(borderTint); 

        this._check1.setActive(self._state).setVisible(self._state);
        this._check2.setActive(self._state).setVisible(self._state);
    }

    Layout(x, y) {
        const self = this;

        self._x = x;
        self._y = y;

        const width = UIGlobals.Sizes.CheckboxSize;
        const height = UIGlobals.Sizes.CheckboxSize;
        const border = UIGlobals.Sizes.CheckboxBorder;

        self._border.setPosition(x, y);
        self._border.setScale(width, height);

        self._background.setPosition(x + border, y + border);
        self._background.setScale(width - border * 2, height - border * 2);

        self._text.setPosition(x + width + UIGlobals.Sizes.CheckboxMargin, y);

        self._check1.setPosition(Math.floor(x + width / 2), Math.floor(y + width / 2));
        self._check2.setPosition(Math.floor(x + width / 2), Math.floor(y + width / 2));
        self._check1.setScale(1, height - height / 4);
        self._check2.setScale(1, height - height / 4);
        self._check1.setAngle(45);
        self._check2.setAngle(-45);

        self.UpdateColors();
    }
}
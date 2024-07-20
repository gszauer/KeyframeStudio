import UIGlobals from './UIGlobals.js'

export default class UIToggle {
    _scene = null;
    _state = false;
    _x = 0;
    _y = 0;
    _width = 0; // only set in layout
    _text = null;
    _border = null;
    _background = null;
    _check1 = null;
    _check2 = null;
    onToggle = null; // arguments: bool value, UIToggle this
    _visible = true;

    _enabled = true;

    constructor(scene, text="", _onToggle = null) {
        this._scene = scene;
        const self = this;

        self.onToggle = _onToggle;

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

        self._text = scene.add.bitmapText(0, 0, UIGlobals.Font100, "");
        self._text.setDepth(UIGlobals.WidgetLayer);
        self._text.text = text;

        borderSprite.setInteractive();
        borderSprite.on("pointerover", function (pointer, localX, localY, event) {
            if (!self._enabled) {
                return;
            }
            if (UIGlobals.Active == null) {
                UIGlobals.Hot = borderSprite;
            }

            self.UpdateColors();
        });
        borderSprite.on("pointerout", function (pointer, event) {
            if (!self._enabled) {
                return;
            }
            if (UIGlobals.Hot == borderSprite) {
                UIGlobals.Hot = null;
            }

            self.UpdateColors();
        });
        borderSprite.on("pointerdown", function (pointer, localX, localY, event) {
            if (!self._enabled) {
                return;
            }
            UIGlobals.Active = borderSprite;

            self.UpdateColors();
        });

        scene.input.on("pointerup", function(pointer, currentlyOver) {
            if (!self._enabled) {
                return;
            }
            if (UIGlobals.Active != null && UIGlobals.Active == borderSprite) {
                let left = borderSprite.x;
                let right = left + borderSprite.scaleX;
                let top = borderSprite.y;
                let bottom = top + borderSprite.scaleY;

                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    self._state = !self._state;
                    //console.log("Toggle " + self._text.text + " to " + self._state);
                    if (self.onToggle != null) {
                        self.onToggle(self._state, self);
                    }
                }
                
                UIGlobals.Active = null;
                self.UpdateColors();
            }
        });
    }

    SetState(value) {
        this._state = !this.value;
        if (this.onToggle != null) {
            this.onToggle(this._state, this);
        }
        this.UpdateColors();
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

        if (!this._enabled) {
            backgroundTint = UIGlobals.Colors.BackgroundLayer1;
            borderTint = UIGlobals.Colors.IconDisabled;
        }
        
        self._text.setTint(borderTint);


        self._border.setTint(borderTint);
        self._background.setTint(backgroundTint);
        self._check1.setTint(borderTint);
        self._check2.setTint(borderTint); 

        this._check1.setActive(this._visible && self._state).setVisible(this._visible && self._state);
        this._check2.setActive(this._visible && self._state).setVisible(this._visible && self._state);
    }

    setPosition(x, y) {
        this.Layout(x, y);
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
        self._check1.setScale(border * 2, height - height / 4);
        self._check2.setScale(border * 2, height - height / 4);
        self._check1.setAngle(45);
        self._check2.setAngle(-45);

        self._width = width + UIGlobals.Sizes.CheckboxMargin + self._text.width;

        self.UpdateColors();
    }

    SetVisibility(value) {
        this._visible = value;
        this._background.setActive(value).setVisible(value);
        this._border.setActive(value).setVisible(value);
        this._check1.setActive(value).setVisible(value);
        this._check2.setActive(value).setVisible(value);
        this._text.setActive(value).setVisible(value);
    }

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

    Disable() {
        this._enabled = false;
        this.UpdateColors();
    }

    Enable() {
        this._enabled = true;
        this.UpdateColors();
    }
}
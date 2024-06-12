import UIGlobals from './UIGlobals.js'
import UIColorPicker from './UIColorPicker.js'

export default class UIColorButton {
    _scene = null;

    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    color = 0;
    onClick = null;

    _colorPicker = null;

    _border = null;
    _background = null;
    _color = null;

    constructor(scene, color = 0xffffff, onClick = null) {
        this._scene = scene;
        this.color = color;
        this.onClick = onClick;

        const self = this;

        self._border = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._border.setOrigin(0, 0);
        self._border.setDepth(UIGlobals.WidgetLayer);

        self._background = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._background.setOrigin(0, 0);
        self._background.setDepth(UIGlobals.WidgetLayer);

        self._color = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._color.setOrigin(0, 0);
        self._color.setDepth(UIGlobals.WidgetLayer);

        this.UpdateColors();

        self._border.setInteractive();
        self._border.on("pointerover", function (pointer, localX, localY, event) {
            if (UIGlobals.Active == null) {
                UIGlobals.Hot = self._border;
            }

            self.UpdateColors();
        });
        self._border.on("pointerout", function (pointer, event) {
            if (UIGlobals.Hot == self._border) {
                UIGlobals.Hot = null;
            }

            self.UpdateColors();
        });

        self._border.on("pointerdown", function (pointer, localX, localY, event) {
            UIGlobals.Active = self._border;

            self.UpdateColors();
        });

        scene.input.on("pointerup", function(pointer, currentlyOver) {
            if (UIGlobals.Active != null && UIGlobals.Active == self._border) {
                let left = self._x;;
                let right = left + self._width;
                let top = self._y;
                let bottom = top + self._height;

                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    if (self.onClick != null) {
                        self.onClick();
                    }
                    self._colorPicker.Show();
                }
                
                UIGlobals.Active = null;
                self.UpdateColors();
            }
        });

        this._colorPicker = new UIColorPicker(scene);
        this._colorPicker.Hide();
    }

    UpdateColors() {
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

        this._border.setTint(borderTint);
        this._background.setTint(backgroundTint);
        this._color.setTint(this.color);
    }

    Layout(x, y, width, height) {
        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        const border = UIGlobals.Sizes.ColorButtonBorderSize;
        const margin = UIGlobals.Sizes.ColorButtonMarginSize
        const picker = UIGlobals.Sizes.ColorPickerDefaultSize;

        this._border.setPosition(x, y);
        this._border.setScale(width, height);

        x += border;
        y += border;
        width -= border * 2;
        height -= border * 2;

        this._background.setPosition(x, y);
        this._background.setScale(width, height);

        x += margin;
        y += margin;
        width -= margin * 2;
        height -= margin * 2;

        this._color.setPosition(x, y);
        this._color.setScale(width, height);

        const centerX = Math.floor(this._x + this._width * 0.5);
        const centerY = Math.floor(this._y + this._height * 0.5);
        this._colorPicker.Layout(Math.floor(centerX - picker * 0.5), Math.floor(centerY - picker * 0.5));
    }

    Show() {
        this._border.setActive(true).setVisible(true);
        this._background.setActive(true).setVisible(true);
        this._color.setActive(true).setVisible(true);
    }

    Hide() {
        if (UIGlobals.Active == self._border) {
            UIGlobals.Active = null;
        }
        if (UIGlobals.Hot == self._border) {
            UIGlobals.Hot = null;
        }
        this._border.setActive(false).setVisible(false);
        this._background.setActive(false).setVisible(false);
        this._color.setActive(false).setVisible(false);
        this._colorPicker.Hide();
    }
}
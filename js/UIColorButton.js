import UIGlobals from './UIGlobals.js'
import UIColorPicker from './UIColorPicker.js'
import ColorRGB from './ColorRGB.js'

export default class UIColorButton {
    _scene = null;

    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    onColorChanged = null; // onColorChanged(rgb);
    onTintChanged = null; // onTintChanged(rgb32)

    _colorPicker = null;

    _border = null;
    _background = null;
    _color = null;

    get color() {
        return this._colorPicker.rgb;
    }


    constructor(scene, tintChanged = null) {
        this._scene = scene;
        this.onTintChanged = tintChanged;

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

        this._colorPicker = new UIColorPicker(scene);
        this._colorPicker.Hide();
        this._colorPicker.onChange = function(newcolor) {
            if (self.onColorChanged != null) {
                self.onColorChanged(self.color);
            }
            if (self.onTintChanged) {
                self.onTintChanged(self.color.color);
            }
            self.UpdateColors();
            //console.log("Color changed to: " + self.color.color);
        }

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
                    self._colorPicker.Show();
                }
                
                UIGlobals.Active = null;
                self.UpdateColors();
            }
            else {
                self._colorPicker._handlePointerUp(pointer, currentlyOver);
            }
        });

        this.UpdateColors();
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
        this._color.setTint(this.color.color);
    }

    Layout(x, y, width, height) {
        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        const border = UIGlobals.Sizes.ColorButtonBorderSize;
        const margin = UIGlobals.Sizes.ColorButtonMarginSize
        const pickerW = UIGlobals.Sizes.ColorPickerDefaultWidth;
        const pickerH = UIGlobals.Sizes.ColorPickerDefaultHeight;

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
        this._colorPicker.Layout(Math.floor(centerX - pickerW * 0.5), Math.floor(centerY - pickerH * 0.5));
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
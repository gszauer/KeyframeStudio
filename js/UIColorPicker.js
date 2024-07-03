import UIGlobals from './UIGlobals.js'
import ColorRGB from './ColorRGB.js'
import ColorHSV from './ColorHSV.js'

export default class UIColorPicker {
    _scene = null;
    onChange = null; // onChange(rgb); <- called like that

    _x = 0;
    _y = 0;
    _height = 0;
    _width = 0;

    _borderSprite = null;
    _backgroundSprite = null;

    _gradientSprites = null;

    _hueTrack = null; // Interactive
    _hueSprites = null;

    _hsvTrack = null; // Interactive
    _currentHsv = null;

    _valueIndicator1 = null;
    _valueIndicator2 = null;

    _hueIndicator1 = null;
    _hueIndicator2 = null;

    _dragging = false;
    _visible = true;

    _handlePointerUp = null; // This is for the owner to call! 

    get hsv() {
        return this._currentHsv;
    }

    get rgb() {
        return this._currentHsv.rgb;
    }

    set rgb(value) {
        this._currentHsv = value.hsv; 
    }

    get color() {
        return this._currentHsv.color;
    }

    constructor(scene, onChange = null, color = null) {
        this._scene = scene; 
        this.onChange = onChange;
        this._currentHsv = color;
        this._currentHsv = new ColorRGB(1, 1, 1).hsv;

        this._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._borderSprite.setDepth(UIGlobals.OverlayLayer);
        this._borderSprite.setOrigin(0, 0);
        this._borderSprite.setTint(UIGlobals.Colors.BorderFraming);

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.OverlayLayer);
        this._backgroundSprite.setOrigin(0, 0);
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);

        this._hueTrack = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._hueTrack.setDepth(UIGlobals.OverlayLayer);
        this._hueTrack.setOrigin(0, 0);
        this._hueTrack.setTint(UIGlobals.Colors.BackgroundLayer1);
        const hueTrack = this._hueTrack;

        this._hsvTrack = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._hsvTrack.setDepth(UIGlobals.OverlayLayer);
        this._hsvTrack.setOrigin(0, 0);
        this._hsvTrack.setTint(UIGlobals.Colors.BackgroundLayer1);
        const hsvTrack = this._hsvTrack;

        this._hueSprites = [];
        for (let i = 0; i < 10; ++i) {
            const sprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            sprite.setDepth(UIGlobals.OverlayLayer);
            sprite.setOrigin(0, 0);
            this._hueSprites.push(sprite);
        }

        this._gradientSprites = [];
        for (let i = 0; i < 5 * 5; ++i) {
            const sprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            sprite.setDepth(UIGlobals.OverlayLayer);
            sprite.setOrigin(0, 0);
            this._gradientSprites.push(sprite);
        }

        const hueTrackWidth = UIGlobals.Sizes.ColorPickerHueTrackWidth;

        this._hueIndicator2 = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._hueIndicator2.setDepth(UIGlobals.OverlayLayer);
        this._hueIndicator2.setOrigin(0, 0);
        this._hueIndicator2.setScale(hueTrackWidth, 4);
        this._hueIndicator2.setTint(0x000000);
        
        this._hueIndicator1 = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._hueIndicator1.setDepth(UIGlobals.OverlayLayer);
        this._hueIndicator1.setOrigin(0, 0);
        this._hueIndicator1.setScale(hueTrackWidth, 2);

        this._valueIndicator2 = scene.add.bitmapText(0, 0, UIGlobals.Font100, name);
        this._valueIndicator2.setDepth(UIGlobals.OverlayLayer);
        this._valueIndicator2.setScale(1.2, 1.2);
        this._valueIndicator2.text = "o";
        this._valueIndicator2.setTint(0x000000);

        this._valueIndicator1 = scene.add.bitmapText(0, 0, UIGlobals.Font100, name);
        this._valueIndicator1.setDepth(UIGlobals.OverlayLayer);
        this._valueIndicator1.text = "o";

        const self = this;
        hueTrack.setInteractive();
        scene.input.setDraggable(hueTrack);

        hsvTrack.setInteractive();
        scene.input.setDraggable(hsvTrack);

        // Need an on click maybe?
        /*scene.input.on("pointerup", function(pointer, currentlyOver)*/this._handlePointerUp = function(pointer, currentlyOver) {
            if (self._visible) {
                let left = hueTrack.x;
                let right = left + hueTrack.scaleX;
                let top = hueTrack.y;
                let bottom = top + hueTrack.scaleY;
                let hide = false;

                // Inside the hue selector
                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    self.hsv.h = ((pointer.y - hueTrack.y) / hueTrack.scaleY) * 359.0;
                    self.hsv.normalize();

                    if (self.onChange != null) {
                        self.onChange(self.rgb);
                    }
                    self.Layout(self._x, self._y);
                }

                left = hsvTrack.x;
                right = left + hsvTrack.scaleX;
                top = hsvTrack.y;
                bottom = top + hsvTrack.scaleY;

                // Inside the value selector
                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    hide = true;

                    self.hsv.s = (pointer.x - hsvTrack.x) / hsvTrack.scaleX;
                    self.hsv.v = (pointer.y - hsvTrack.y) / hsvTrack.scaleY;
                    self.hsv.normalize();

                    if (self.onChange != null) {
                        self.onChange(self.rgb);
                    }
                    self.Layout(self._x, self._y);
                }

                left = self._x;
                right = left + self._width;
                top = self._y;
                bottom = top + self._height;

                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    // Clicked inside
                }
                else {
                    hide = true;
                }

                if (hide) {
                    self.Hide();
                }

                if (UIGlobals.Active == hueTrack || UIGlobals.Active == hsvTrack) {
                    UIGlobals.Active = null;
                }
            }
        };//);

        scene.input.on('dragstart', (pointer, gameObject) => {
            if (gameObject != hueTrack && gameObject != hsvTrack) {
                return;
            }
            
            UIGlobals.Active = gameObject;
            self._dragging = true;
        });

        scene.input.on('dragend', (pointer, gameObject) => {
            if (gameObject != hueTrack && gameObject != hsvTrack) {
                return;
            }

            if (UIGlobals.Active == hueTrack || UIGlobals.Active == hsvTrack) {
                UIGlobals.Active = null;
            }

            if (gameObject == hsvTrack) {
                self._handlePointerUp(pointer, [gameObject]);
                self.Hide();
            }
            self._dragging = false;
        });

        scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
            if (gameObject != hueTrack && gameObject != hsvTrack) {
                return;
            }

            UIGlobals.Active = gameObject;

            if (pointer.x < gameObject.x) { pointer.x = gameObject.x; }
            if (pointer.x > gameObject.x + gameObject.scaleX) { pointer.x = gameObject.x + gameObject.scaleX; }
            if (pointer.y < gameObject.y) { pointer.y = gameObject.y; }
            if (pointer.y > gameObject.y + gameObject.scaleY) { pointer.y = gameObject.y + gameObject.scaleY; }

            if (gameObject == hueTrack) {
                const t = (pointer.y - gameObject.y) / gameObject.scaleY;
                if (t < 0) { t = 0; } 
                if (t > 1) { t = 1; }
                self.hsv.h = t * 359.0;
            }

            if (gameObject == hsvTrack) {
                self.hsv.s = (pointer.x - gameObject.x) / gameObject.scaleX;
                self.hsv.v = (pointer.y - gameObject.y) / gameObject.scaleY;
                //self.hsv.v = 1.0 - self.hsv.v;
            }
            
            self.Layout(self._x, self._y);
            if (self.onChange != null) {
                self.onChange(self.rgb);
            }
        });
    }

    Layout(x, y) {
        const border = UIGlobals.Sizes.PopupMenuBorderSize;
        const margin = UIGlobals.Sizes.ColorPickerInnerMargin;
        const defaultWidth = UIGlobals.Sizes.ColorPickerDefaultWidth;
        const defaultHeight = UIGlobals.Sizes.ColorPickerDefaultHeight;
        const hueTrackWidth = UIGlobals.Sizes.ColorPickerHueTrackWidth;

        this._x = x;
        this._y = y;
        let width = this._width = defaultWidth;
        let height = this._height = defaultHeight;

        this._borderSprite.setPosition(x, y);
        this._borderSprite.setScale(width, height);

        x += border;
        y += border;
        width -= border * 2;
        height -= border * 2;

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(width, height);

        x += margin;
        y += margin;
        width -= margin * 2;
        height -= margin * 2;

        const sliceSize = Math.floor(height / 10.0);
        //console.log("_height: " + this._height + ", height: " + height + ", slice: " + sliceSize + ", (" + sliceSize + " * 10): " + (sliceSize * 10));
        //console.log("width: " + (this._height + margin * 3 + border * 2));
        
        const hsvTop = new ColorHSV(359, 1, 1);
        const hsvBottom = new ColorHSV(0, 1, 1);

        this._hueTrack.setPosition(x, y);
        this._hueTrack.setScale(hueTrackWidth, height);

        this._hueIndicator1.setPosition(x, y + (this.hsv.h / 359.0) * height);
        this._hueIndicator2.setPosition(x, y + (this.hsv.h / 359.0) * height - 1);

        for (let i = 0; i < 10; ++i) {
            const t0 = (i) / 10.0;
            const t1 = (i + 1) / 10.0;

            hsvTop.h = 359.0 * t0;
            hsvBottom.h = 359.0 * t1;

            const rgbTop = hsvTop.rgb;
            const rgbBottom = hsvBottom.rgb;

            const sprite = this._hueSprites[i];
            sprite.setPosition(x, y + sliceSize * i);
            sprite.setScale(hueTrackWidth, sliceSize);

            sprite.setTintFill(
                rgbTop.color, rgbTop.color, 
                rgbBottom.color, rgbBottom.color);
        }

        x += hueTrackWidth;
        x += margin;
        width -= hueTrackWidth;
        width -= margin;

        const bottomLeft = new ColorHSV(this.hsv.h, 0, 1);
        const bottomRight = new ColorHSV(this.hsv.h, 1, 1);
        const topLeft = new ColorHSV(this.hsv.h, 0, 0);
        const topRight = new ColorHSV(this.hsv.h, 1, 0);

        const pickAreaWidth = height;
        const pickAreHeight = height;
        const slice_w = Math.floor(pickAreaWidth / 5.0);
        const slice_h = Math.floor(pickAreHeight / 5.0);

        this._hsvTrack.setPosition(x, y);
        this._hsvTrack.setScale(height, height); // NOT A TYPO, IT'S SQUARE!

        for (let col = 0; col < 5; ++col) {
            const x0 = col / 5.0;
            const x1 = (col + 1) / 5.0;

            const topInterpolated = topLeft.lerp(topRight, x0)

            for (let row = 0; row < 5; ++row) {
                const sprite = this._gradientSprites[col + row * 5];

                const y0 = row / 5.0;
                const y1 = (row + 1) / 5.0;

                const _tl = topLeft.lerp(topRight, x0).lerp(bottomLeft.lerp(bottomRight, x0), y0).color;
                const _tr = topLeft.lerp(topRight, x1).lerp(bottomLeft.lerp(bottomRight, x1), y0).color;
                const _bl = topLeft.lerp(topRight, x0).lerp(bottomLeft.lerp(bottomRight, x0), y1).color;
                const _br = topLeft.lerp(topRight, x1).lerp(bottomLeft.lerp(bottomRight, x1), y1).color;

                sprite.setPosition(x + col * slice_w, y + row * slice_h);
                sprite.setScale(slice_w, slice_h);
                sprite.setTintFill(_tl, _tr, _bl, _br);
            }
        }

        const offsetX = this._valueIndicator1.width * 0.5;
        const offsetY = this._valueIndicator1.height * 0.5 + this._valueIndicator1.height * 0.25;
        const pointerX = x + this.hsv.s * height; // Again, not typo, square
        const pointerY = y + (this.hsv.v) * height;
        this._valueIndicator1.setPosition(pointerX - offsetX, pointerY - offsetY);
        this._valueIndicator2.setPosition(pointerX - offsetX * 1.2, pointerY - offsetY * 1.2);
    }

    SetVisibility(visible) {
        this._borderSprite.setActive(visible).setVisible(visible);
        this._backgroundSprite.setActive(visible).setVisible(visible);
        this._hueTrack.setActive(visible).setVisible(visible);
        this._hsvTrack.setActive(visible).setVisible(visible);
        this._valueIndicator1.setActive(visible).setVisible(visible);
        this._valueIndicator2.setActive(visible).setVisible(visible);
        this._hueIndicator1.setActive(visible).setVisible(visible);
        this._hueIndicator2.setActive(visible).setVisible(visible);
        for (let i = 0; i < 10; ++i) {
            this._hueSprites[i].setActive(visible).setVisible(visible);
        }
        for (let i = 0; i < 5 * 5; ++i) {
            this._gradientSprites[i].setActive(visible).setVisible(visible);
        }
    }

    Show() {
        this._visible = true;
        this.SetVisibility(true);
    }

    Hide() {
        this._visible = false;
        this.SetVisibility(false);
    }
}
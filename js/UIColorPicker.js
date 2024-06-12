import UIGlobals from './UIGlobals.js'
import ColorRGB from './ColorRGB.js'
import ColorHSV from './ColorHSV.js'

export default class UIColorPicker {
    _scene = null;
    onChange = null; // onChange(0xffff00); <- called like that

    _x = 0;
    _y = 0;
    _height = 0;
    _width = 0;

    _borderSprite = null;
    _backgroundSprite = null;

    _gradientSprites = null;

    _hueTrack = null;
    _hueSprites = null;

    constructor(scene) {
        this._scene = scene; 

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

        this._hueSprites = [];
        for (let i = 0; i < 10; ++i) {
            const sprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            sprite.setDepth(UIGlobals.OverlayLayer);
            sprite.setOrigin(0, 0);
            this._hueSprites.push(sprite);
        }
    }

    Layout(x, y) {
        const border = UIGlobals.Sizes.PopupMenuBorderSize;
        const margin = UIGlobals.Sizes.ColorPickerInnerMargin;
        const defaultSize = UIGlobals.Sizes.ColorPickerDefaultSize;

        this._x = x;
        this._y = y;
        this._width = defaultSize;
        this._height = defaultSize;
        let width = defaultSize;
        let height = defaultSize;

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
        
        const hsvTop = new ColorHSV(359, 1, 1);
        const hsvBottom = new ColorHSV(0, 1, 1);

        const hueTrackWidth = UIGlobals.Sizes.ColorPickerHueTrackWidth;

        this._hueTrack.setPosition(x, y);
        this._hueTrack.setScale(hueTrackWidth, height);

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
    }

    SetVisibility(visible) {
        this._borderSprite.setActive(visible).setVisible(visible);
        this._backgroundSprite.setActive(visible).setVisible(visible);
        this._hueTrack.setActive(visible).setVisible(visible);
        for (let i = 0; i < 10; ++i) {
            this._hueSprites[i].setActive(visible).setVisible(visible);
        }
    }

    Show() {
        this.SetVisibility(true);
    }

    Hide() {
        this.SetVisibility(false);
    }
}
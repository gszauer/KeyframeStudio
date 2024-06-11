import UIGlobals from './UIGlobals.js'

// https://csdgn.org/article/scrollbar
export default class UIScrollBar {
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    _trackX = 0;
    _trackY = 0;
    _trackWidth = 0;
    _trackHeight = 0;
    _trackScroll = 0; // scrollable width or height

    _gripX = 0;
    _gripY = 0;
    _gripWidth = 0;
    _gripHeight = 0;

    horizontal = false;
    onScroll = null; // OnScroll(float normalized);
    current = 0; // 0 to 1 depending on current scroll

    _buttonASprite = null;
    _buttonBSprite = null;
    _trackSprite = null;
    _gripSprite = null;

    constructor(scene) {
        this._trackSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._trackSprite.setOrigin(0, 0);
        this._trackSprite.setDepth(UIGlobals.WidgetLayer);

        this._buttonASprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._buttonASprite.setOrigin(0, 0);
        this._buttonASprite.setDepth(UIGlobals.WidgetLayer);
        
        this._buttonBSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._buttonBSprite.setOrigin(0, 0);
        this._buttonBSprite.setDepth(UIGlobals.WidgetLayer);
        
        this._gripSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._gripSprite.setOrigin(0, 0);
        this._gripSprite.setDepth(UIGlobals.WidgetLayer);
    }

    Hide() {
        this._buttonASprite.setActive(false).setVisible(false);
        this._buttonBSprite.setActive(false).setVisible(false);
        this._trackSprite.setActive(false).setVisible(false);
        this._gripSprite.setActive(false).setVisible(false);
    }

    Show() {
        this._buttonASprite.setActive(true).setVisible(true);
        this._buttonBSprite.setActive(true).setVisible(true);
        this._trackSprite.setActive(true).setVisible(true);
        this._gripSprite.setActive(true).setVisible(true);
    }

    UpdateColors() {
        this._buttonASprite.setTint(0xff0000);
        this._buttonBSprite.setTint(0xff0000);
        this._trackSprite.setTint(0x0000ff);
        this._gripSprite.setTint(0x00ff00);
    }
    
    Layout(x, y, width, height, contentRatio) {
        if (contentRatio === null || contentRatio === undefined) {
            throw Error("cotnent ratio is not optional for UIScrollBar");
        }
        const scrollTrackSize = UIGlobals.Sizes.ScrollTrackSize;
        const horizontal = this.horizontal;
        const minGripSize = UIGlobals.Sizes.ScrollTrackMinGripSize;

        if (contentRatio < 0) {
            contentRatio = 0;
        }
        if (contentRatio > 1) {
            contentRatio = 1;
        }

        if (horizontal) {
            height = scrollTrackSize;
            if (width < scrollTrackSize * 4) {
                width = scrollTrackSize * 4;
            }
        }
        else {
            width = scrollTrackSize;
            if (height < scrollTrackSize * 4) {
                height = scrollTrackSize * 4;
            }
        }

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._buttonASprite.setPosition(x, y);
        this._buttonASprite.setScale(scrollTrackSize, scrollTrackSize);

        if (horizontal) {
            x += scrollTrackSize;
            width -= scrollTrackSize * 2;
        }
        else {
            y += scrollTrackSize;
            height -= scrollTrackSize * 2;
        }

        this._trackX = x;
        this._trackY = y;
        this._trackWidth = width;
        this._trackHeight = height;

        this._trackSprite.setPosition(x, y);
        this._trackSprite.setScale(width, height);

        let gripWidth = scrollTrackSize;
        let gripHeight = scrollTrackSize;

        if (horizontal) {
            gripWidth = width * contentRatio;
            if (gripWidth < minGripSize) {
                gripWidth = minGripSize;
            }
        }
        else {
            gripHeight = height * contentRatio;
            if (gripHeight < minGripSize) {
                gripHeight = minGripSize;
            }
        }

        this._gripX = x;
        this._gripY = y;
        this._gripWidth = gripWidth;
        this._gripHeight = gripHeight;

        this._gripSprite.setPosition(x, y);
        this._gripSprite.setScale(gripWidth, gripHeight);

        if (horizontal) {
            x = this._x + this._width - scrollTrackSize;
        }
        else {
            y = this._y + this._height - scrollTrackSize;
        }

        this._buttonBSprite.setPosition(x, y);
        this._buttonBSprite.setScale(scrollTrackSize, scrollTrackSize);

        this.UpdateColors();
    }
}
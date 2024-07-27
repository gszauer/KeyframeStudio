import UIImageButton from './UIImageButton.js'
import UIGlobals from './UIGlobals.js'
import UIListBox from './UIListBox.js'
import Clip from './Animation.js'
import UIView from './UIView.js'

export default class KeyframesView extends UIView {
    _background = null;
    _header = null;

    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._background = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._background.setDepth(UIGlobals.WidgetLayer);
        this._background.setOrigin(0, 0);

        this._header = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._header.setDepth(UIGlobals.WidgetLayer);
        this._header.setOrigin(0, 0);
    }

    UpdateColors() {
        this._background.setTint(0xff0000);
        this._header.setTint(0x00ff00);
        // TODO
    }

    Layout(x, y, width, height) {
        if (x === undefined) { x = this._x; }
        if (y === undefined) { y = this._y; }
        if (width === undefined) { width = this._width; }
        if (height === undefined) { height = this._height; }

        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        
        super.Layout(x, y, width, height);

        const headerHeight = UIGlobals.Sizes.KeyframeViewHeaderHeight;

        this._background.setPosition(x, y + headerHeight);
        this._background.setScale(width, height - headerHeight);

        this._header.setPosition(x, y);
        this._header.setScale(width, headerHeight);

        // TODO
    }

    SetVisibility(value) {
        this._background.setActive(value).setVisible(value);
        this._header.setActive(value).setVisible(value);
        // TODO
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
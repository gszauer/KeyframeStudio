import UIGlobals from './UIGlobals.js'

export default class UIListBoxItem {
    _scene = null;
    _owner = null;

    visible = true;

    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    _backgroundSprite = null;
    _labelText = null;

    constructor(parentListBox, text = "") {
        this._scene = parentListBox._scene;
        this._owner = parentListBox;

        this._backgroundSprite = this._scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);

        this._labelText = this._scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        this._labelText.setDepth(UIGlobals.WidgetLayer);
        this._labelText.setOrigin(0, 0);
        this._labelText.text = text;
        this._labelText.setTint(0);
    }

    Destroy() {
        this._owner.Remove(this);
        this._backgroundSprite.destroy();
        this._backgroundSprite = null;
        this._labelText.destroy();
        this._labelText = null;
    }

    UpdateText(text = "") {
        this._labelText.text = text;
    }

    AddToContainer(container) {
        container.add(this._backgroundSprite);
        container.add(this._labelText);
    }

    Layout(x, y, width) {
        if (width < 0) { width = 0; }
        const height = UIGlobals.Sizes.ListBoxItemHeight;
        const marginLeft = UIGlobals.Sizes.ListBoxItemMarginLeft;

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(width, height);

        this._labelText.setPosition(x + marginLeft, y);
    }

    SetTint(background = 0x000000, text = 0xffffff) {
        this._backgroundSprite.setTint(background);
        this._labelText.setTint(text);
    }
    
    SetVisibility(value) {
        this._backgroundSprite.setActive(value).setVisible(value);
        this._labelText.setActive(value).setVisible(value);
        this.visible = value;
    }

    Show() {
        this.SetVisibility(true);
    }

    Hide() {
        this.SetVisibility(false);
    }
}
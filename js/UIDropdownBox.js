import UIConstants from './UIConstants.js'

export default class UIDropdownBox {
    _scene = null;

    _borderSprite = null;
    _backgroundSprite = null;
    _seperatorSprite = null;
    _chevronSprite = null;
    _displayText = null;

    _x = 0;
    _y = 0;
    _width = 0;
    _defaultWidth = 0;

    _popupMenu = null;

    constructor(scene, popupMenu, defaultWidth = 0) {
        this._scene = scene;
        const self = this;

        self._defaultWidth = defaultWidth;

        self._borderSprite = scene.add.sprite(0, 0, UIConstants.Atlas, UIConstants.Solid);
        self._borderSprite.setOrigin(0, 0);
        self._borderSprite.setDepth(UIConstants.WidgetLayer);

        self._backgroundSprite = scene.add.sprite(0, 0, UIConstants.Atlas, UIConstants.Solid);
        self._backgroundSprite.setOrigin(0, 0);
        self._backgroundSprite.setDepth(UIConstants.WidgetLayer);

        self._seperatorSprite = scene.add.sprite(0, 0, UIConstants.Atlas, UIConstants.Solid);
        self._seperatorSprite.setOrigin(0, 0);
        self._seperatorSprite.setDepth(UIConstants.WidgetLayer);

        self._chevronSprite = scene.add.sprite(0, 0, UIConstants.Atlas, UIConstants.Icons.ChevronDown);
        self._chevronSprite.setDepth(UIConstants.WidgetLayer);

        self._displayText = scene.add.bitmapText(0, 0, UIConstants.Font100, name);
        self._displayText.setDepth(UIConstants.WidgetLayer);
        self._displayText.text = "poopy butthole";

        // TODO: Create Label
        self.SetMenu(popupMenu);
    }

    SetMenu(popupMenu) {

    }
    
    Layout(x, y, width = 0) {
        this._x = x;
        this._y = y;
        this._width = width;
        if (this._defaultWidth > 0 && this._width <= 0) {
            this._width = this._defaultWidth;
        }
        const height = UIConstants.Sizes.DropdownMenuHeight;
        const border = UIConstants.Sizes.DrowdownBorderSize;

        const idleTint = UIConstants.Colors.Dark.Gray600;

        this._borderSprite.setPosition(x, y);
        this._borderSprite.setTint(idleTint);
        this._borderSprite.setScale(this._width, height);

        this._backgroundSprite.setPosition(x + border, y + border);
        this._backgroundSprite.setTint(UIConstants.Colors.BackgroundLayer1);
        this._backgroundSprite.setScale(this._width - border * 2, height - border * 2);

        this._seperatorSprite.setPosition(this._x + this._width - border * 2 - height, y );
        this._seperatorSprite.setTint(idleTint);
        this._seperatorSprite.setScale(border, height);

        const chevronX = this._x + this._width - border  - height / 2;
        const chevronY = y + border + height / 2;
        this._chevronSprite.setPosition(chevronX, chevronY);
        this._chevronSprite.setTint(idleTint);

        this._displayText.setTint(idleTint);
        this._displayText.setPosition(this._x + UIConstants.Sizes.DropdownTextIndent, this._y, chevronY);
    }

    Destroy() {

    }
}
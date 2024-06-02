import UIGlobals from './UIGlobals.js'

export default class UIDropdown {
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
    _isOpen = false;

    _popupMenu = null;

    constructor(scene, popupMenu, defaultWidth = 0) {
        this._scene = scene;
        const self = this;

        self._defaultWidth = defaultWidth;

        self._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._borderSprite.setOrigin(0, 0);
        self._borderSprite.setDepth(UIGlobals.WidgetLayer);
        const borderSprite = self._borderSprite;

        self._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._backgroundSprite.setOrigin(0, 0);
        self._backgroundSprite.setDepth(UIGlobals.WidgetLayer);

        self._seperatorSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._seperatorSprite.setOrigin(0, 0);
        self._seperatorSprite.setDepth(UIGlobals.WidgetLayer);

        self._chevronSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Icons.ChevronDown);
        self._chevronSprite.setDepth(UIGlobals.WidgetLayer);

        self._displayText = scene.add.bitmapText(0, 0, UIGlobals.Font100, name);
        self._displayText.setDepth(UIGlobals.WidgetLayer);
        self._displayText.text = "";

        self.SetMenu(popupMenu);

        borderSprite.setInteractive();
        borderSprite.on("pointerover", function (pointer, localX, localY, event) {
            if (UIGlobals.Active == null) {
                UIGlobals.Hot = borderSprite;
            }

            self._ShowActiveMenu();
            self.UpdateColors();
        });
        borderSprite.on("pointerout", function (pointer, event) {
            if (UIGlobals.Hot == borderSprite) {
                UIGlobals.Hot = null;
            }

            self._ShowActiveMenu();
            self.UpdateColors();
        });
        borderSprite.on("pointerdown", function (pointer, localX, localY, event) {
            UIGlobals.Active = borderSprite;
            self._isOpen = true;

            self._ShowActiveMenu();
            self.UpdateColors();
        });

        scene.input.on("pointerup", function(pointer, currentlyOver) {
            if (self._popupMenu != null) {
                self._popupMenu.HandlePointerUpEvent(pointer, currentlyOver);
            }

            if (UIGlobals.Active != null && UIGlobals.Active == borderSprite) {
                let left = borderSprite.x;
                let right = left + borderSprite.scaleX;
                let top = borderSprite.y;
                let bottom = top + borderSprite.scaleY;

                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    self._isOpen = true;
                }
                else {
                    self._isOpen = false;
                }
                
                UIGlobals.Active = null;
                self._ShowActiveMenu();
                self.UpdateColors();
            }
            else if (self._isOpen) {
                self._isOpen = false;
                self._ShowActiveMenu();
                self.UpdateColors();
            }
        });
    }

    SetMenu(popupMenu) {
        if (this._popupMenu != null) {
            this._popupMenu._uiOnPointerUp = null;
        }

        this._popupMenu = popupMenu;
        this._isOpen = false;

        if (this._popupMenu != null) {
            const self = this;
            this._popupMenu._uiOnPointerUp = function(strang) {
                self._displayText.text = strang;
            }
            self._displayText.text = "";
            if (this._popupMenu._labels != null && this._popupMenu._labels.length > 0) {
                self._displayText.text = this._popupMenu._labels[0];
            }

        }

        this._ShowActiveMenu();
        this.UpdateColors();
    }

    _ShowActiveMenu() {
        if (this._isOpen) {
            if (this._popupMenu != null) {
                this._popupMenu.Show();
            }
        }
        else {
            if (this._popupMenu != null) {
                this._popupMenu.Hide();
            }
        }
    }

    UpdateColors() {
        let borderTint = UIGlobals.Colors.ElementBorderTintIdle;
        let backgroundTint = UIGlobals.Colors.BackgroundLayer1;

        if (UIGlobals.Hot == this._borderSprite) {
            borderTint = UIGlobals.Colors.ElementBorderTintHot;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }
        if (UIGlobals.Active == this._borderSprite || this._isOpen) {
            borderTint = UIGlobals.Colors.ElementBorderTintActive;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }

        this._borderSprite.setTint(borderTint);
        this._seperatorSprite.setTint(borderTint);
        
        this._backgroundSprite.setTint(backgroundTint);
        
        this._displayText.setTint(borderTint);
        this._chevronSprite.setTint(borderTint);

    }
    
    Layout(x, y, width = 0) {
        this._x = Math.floor(x);
        this._y = Math.floor(y);
        this._width = Math.floor(width);

        if (this._defaultWidth > 0 && this._width <= 0) {
            this._width = Math.floor(this._defaultWidth);
        }
        const height = Math.floor(UIGlobals.Sizes.DropdownMenuHeight);
        const border = Math.floor(UIGlobals.Sizes.DrowdownBorderSize);

        this._borderSprite.setPosition(x, y);
        this._borderSprite.setScale(this._width, height);

        this._backgroundSprite.setPosition(x + border, y + border);
        this._backgroundSprite.setScale(this._width - border * 2, height - border * 2);

        this._seperatorSprite.setPosition(this._x + this._width - border * 2 - height, y );
        this._seperatorSprite.setScale(border, height);

        const chevronX = this._x + this._width - border  - height / 2;
        const chevronY = y + border + height / 2;
        this._chevronSprite.setPosition(chevronX, chevronY);

        this._displayText.setPosition(this._x + UIGlobals.Sizes.DropdownTextIndent, this._y + 1, chevronY);
        this.UpdateColors();

        if (this._popupMenu != null) {
            this._popupMenu.Layout(this._x, this._y + height + border * 2, this._width);
        }
    }
}
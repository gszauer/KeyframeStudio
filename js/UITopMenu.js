import UIConstants from './UIConstants.js'

export default class UITopMenu {
    // Menu transform, always at top of window with constant size
    _width = 0;
    _height = 0;

    _scene = null; // Phaser scene the menu is active in
    _backgroundSprite = null; // Phaser sprite for menu background
    // List of unique phaser labels. Each string is the name of an item in the menu
    _labelTexts = null;
    // Each button has a sprite when hovered over.
    _buttonSprites = null;

    // Dictionary. Key is the name of a menu in the file menu. Value is a UIPopupMenu
    _menu = null;
    _activeMenu = -1;

    constructor(scene) {
        const self = this;

        self._scene = scene;
        self._menu = new Map();
        self._labelTexts = [];
        self._buttonSprites = [];

        self._backgroundSprite = scene.add.sprite(0, 0, UIConstants.Atlas, UIConstants.Solid);
        self._backgroundSprite.setOrigin(0, 0);
        self._backgroundSprite.setPosition(0, 0);
        self._backgroundSprite.setDepth(UIConstants.WidgetLayer);

        scene.input.on("pointerup", function(pointer, currentlyOver) {
            for(const [key, popup] of self._menu) {
                if (popup != null) {
                    popup.HandlePointerUpEvent(pointer, currentlyOver);
                }
            }
            
            if (self._buttonSprites != null && UIConstants.Active != null) {
                let activeFound = false;
                for(const sprite of self._buttonSprites) {
                    if (UIConstants.Active == sprite) {
                        activeFound = true;
                        let left = sprite.x;
                        let right = left + sprite.scaleX;
                        let top = sprite.y;
                        let bottom = top + sprite.scaleY;

                        if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                            self._activeMenu = self._SpriteIndex(sprite);
                        }
                        else { // Clicked outside
                            // Is the mouse in the current active menu?
                            if (self._activeMenu != -1) {
                                const activeMenu = self._menu.get(self._labelTexts[self._activeMenu].text);
                                left = activeMenu._x;
                                top = activeMenu._y;
                                right = left + activeMenu._width;
                                bottom = top + activeMenu._height;

                                if (pointer.x < left || pointer.x > right || pointer.y < top || pointer.y > bottom) {
                                    self._activeMenu = -1;
                                }
                            }
                        }

                        UIConstants.Active = null;
                    }
                }
                if (!activeFound) {
                    self._activeMenu = -1;
                }
            }
            else {
                self._activeMenu = -1;
            }

            self._ShowActiveMenu();
            self._TintButtons();
        });
    }

    _SpriteIndex(sprite) {
        const size = this._buttonSprites.length;
        for (let i = 0; i < size; ++i) {
            if (sprite == this._buttonSprites[i]) {
                return i;
            }
        }
        return -1;
    }

    _ContainsSprite(sprite) {
        for(const button of this._buttonSprites) {
            if (sprite == button) {
                return true;
            }
        }
        return false;
    }

    _ShowActiveMenu() {
        let activeTitle = "";
        if (this._activeMenu >= 0) {
            activeTitle = this._labelTexts[this._activeMenu].text;
        }

        for(const [key, popup] of this._menu) {
            if (popup == null) {
                continue;
            }

            if (key == activeTitle) {
                popup.Show();
            }
            else{
                popup.Hide();
            }
        }
    }

    _TintButtons() {
        const size = this._buttonSprites.length;
        for (let i = 0; i < size; ++i) {
            const buttonSprite = this._buttonSprites[i];
            if (buttonSprite == UIConstants.Active) {
                buttonSprite.setTint(UIConstants.Colors.TopMenuButtonActive);
            }
            else if (this._activeMenu == i) {
                buttonSprite.setTint(UIConstants.Colors.TopMenuButtonActive);
            }
            else if (buttonSprite == UIConstants.Hot) {
                buttonSprite.setTint(UIConstants.Colors.TopMenuButtonHot);
            }
            else {
                buttonSprite.setTint(UIConstants.Colors.TopMenuButtonIdle);
            }
        }
    }

    Add(name, menu) {
        const self = this;
        const scene = self._scene;

        if (self._menu.has(name)) {
            throw new Error("File menu already contains item with key: " + name);
        }
        self._menu.set(name, menu);

        const buttonSprite = scene.add.sprite(0, 0, UIConstants.Atlas, UIConstants.Solid);
        self._buttonSprites.push(buttonSprite);
        buttonSprite.setTint(UIConstants.Colors.TopMenuButtonIdle);
        buttonSprite.setDepth(UIConstants.WidgetLayer);

        buttonSprite.setInteractive();
        buttonSprite.on("pointerover", function (pointer, localX, localY, event) {
            if (UIConstants.Active == null) {
                if (self._activeMenu >= 0) { // Switch active menu to us
                    self._activeMenu = self._SpriteIndex(buttonSprite);
                }
                UIConstants.Hot = buttonSprite;
            }
            else {
                if (self._ContainsSprite(UIConstants.Active)) {
                    UIConstants.Active  = buttonSprite;
                    self._activeMenu = self._SpriteIndex(buttonSprite);
                }
            }

            self._ShowActiveMenu();
            self._TintButtons();
        })
        buttonSprite.on("pointerout", function (pointer, event) {
            if (UIConstants.Hot == buttonSprite) {
                UIConstants.Hot = null;
            }

            self._ShowActiveMenu();
            self._TintButtons();
        });
        buttonSprite.on("pointerdown", function (pointer, localX, localY, event) {
            if (UIConstants.Hot == buttonSprite) {
                //if (UIConstants.Active == null || UIConstants.Active == buttonSprite) {
                    UIConstants.Active = buttonSprite;
                    self._activeMenu = self._SpriteIndex(buttonSprite);
                //}
            }

            self._ShowActiveMenu();
            self._TintButtons();
        });

        const bitmapText = scene.add.bitmapText(0, 0, UIConstants.Font200, name);
        bitmapText.setTint(UIConstants.Colors.Text);
        bitmapText.setDepth(UIConstants.WidgetLayer);

        self._labelTexts.push(bitmapText);
    }

    Destroy() {
        const self = this;

        if (self._menu != null) {
            for(const [key, popup] of this._menu) {
                if (popup != null) {
                    popup.Destroy();
                }
            }
            self._menu = null;
        }

        if (self._labelTexts != null) {
            for(const label of self._labelTexts) {
                if (label != null) {
                    label.destroy();
                }
            }
            self._labelTexts = null;
        }

        if (self._buttonSprites != null) {
            for(const sprite of self._buttonSprites) {
                if (sprite != null) {
                    sprite.destroy();
                }
            }
            self._buttonSprites = null;
        }

        self._backgroundSprite.destroy();
        self._backgroundSprite = null;

        self._width = 0;
        self._height = 0;
        self._scene = null;
        self._activeMenu = -1;
    }

    Layout() {
        const self = this;
       
        self._width = self._scene.scale.width;
        self._height = UIConstants.Sizes.TopMenuHeight;

        self._backgroundSprite.setTint(UIConstants.Colors.BackgroundLayer1);
        self._backgroundSprite.setScale(self._width, self._height);

        let x = UIConstants.Sizes.TopMenuPadding * 2;
        let y = UIConstants.Sizes.TopMenuPadding * 2;

        const size = this._labelTexts.length;

        for(let i = 0; i < size; ++i) {
            const label = self._labelTexts[i];
            const sprite = self._buttonSprites[i];
            const text = self._labelTexts[i].text;

            sprite.setPosition(Math.floor(x), UIConstants.Sizes.TopMenuPadding * 3);
            sprite.setScale(label.width + UIConstants.Sizes.TopMenuMargin * 2, UIConstants.Sizes.TopMenuHeight - UIConstants.Sizes.TopMenuPadding * 6);

            if (self._menu.has(text) && self._menu.get(text) != null) {
                let forceWidth = undefined;
                if (text == "Edit") {
                    forceWidth = 150;
                }

                self._menu.get(text).Layout(Math.floor(x), y + UIConstants.Sizes.TopMenuHeight, forceWidth);
            }

            x += UIConstants.Sizes.TopMenuMargin;
            label.setPosition(Math.floor(x), y);
            x += label.width + UIConstants.Sizes.TopMenuMargin + UIConstants.Sizes.TopMenuPadding * 2;
        }
        
        self._ShowActiveMenu();
        self._TintButtons();
    }
}
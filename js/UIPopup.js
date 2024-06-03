import UIGlobals from './UIGlobals.js'

export default class UIPopup {
    _x = 0;
    _y = 0;
    _height = 100;
    _width = 100;
    _defaultWidth = 100;

    _scene = null;
    _borderSprite = null;
    _backgroundSprite = null;
    _itemTexts = null;
    _highlightSprites = null;
    _seperatorSprites = null;
    _callbacks = null;
    _labels = null; // Just text
    _numLabels = 0;
    _numSeperators = 0;
    _visible = true;

    _uiOnPointerUp = null; // same as regular OnPointerUp, but internal to the ui system
    _onSelect = null; // Arguments: string item, UIPopup self

    constructor(scene, onSelect = null) {
        const self = this;
        self._onSelect = onSelect;

        self._scene = scene;
        self._itemTexts = [];
        self._highlightSprites = [];
        self._seperatorSprites = [];
        self._callbacks = [];
        self._labels = [];

        self._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._borderSprite.setDepth(UIGlobals.OverlayLayer);

        self._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._backgroundSprite.setDepth(UIGlobals.OverlayLayer);
    }

    HandlePointerUpEvent(pointer, currentlyOver) {
        const self = this;

        if (self._highlightSprites != null && UIGlobals.Active != null) {
            const size = self._highlightSprites.length;
            for (let i = 0; i < size; ++i) {
                const sprite = self._highlightSprites[i];

                if (sprite != null && sprite == UIGlobals.Active) {
                    const left = sprite.x;
                    const right = left + sprite.scaleX;
                    const top = sprite.y;
                    const bottom = top + sprite.scaleY;

                    if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                        console.log("Trigger callback for " + self._labels[i]);
                        if (self._uiOnPointerUp != null) {
                            self._uiOnPointerUp(self._labels[i]);
                        }
                        if (self._onSelect != null) {
                            self._onSelect(self._labels[i], self);
                        }
                    }

                    UIGlobals.Active = null;
                    UIGlobals.Hot = null;
                }
            }
        }
        
        self._UpdateHighlights();
    }

    Add(name, callback) {
        const self = this;
        const scene = self._scene;

        self._labels.push(name);
        self._callbacks.push(callback);

        if (name === "") {
            const seperatorSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            seperatorSprite.setDepth(UIGlobals.OverlayLayer);
            self._seperatorSprites.push(seperatorSprite);

            self._highlightSprites.push(null);
            self._itemTexts.push(null);
            self._numSeperators += 1;
        }
        else {
            self._seperatorSprites.push(null);

            const highlightSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            highlightSprite.setDepth(UIGlobals.OverlayLayer);
            highlightSprite.setTint(UIGlobals.Colors.TopMenuButtonIdle);
            self._highlightSprites.push(highlightSprite);

            const itemText = scene.add.bitmapText(0, 0, UIGlobals.Font100, name);
            itemText.setTint(UIGlobals.Colors.Text);
            itemText.setDepth(UIGlobals.OverlayLayer);
            self._itemTexts.push(itemText);
            self._numLabels += 1;


            highlightSprite.setInteractive();
            highlightSprite.on("pointerover", function (pointer, localX, localY, event) {
                UIGlobals.Hot = highlightSprite;
                
                if (self._ContainsHighlight(UIGlobals.Active)) {
                    UIGlobals.Active = highlightSprite;
                }

                // Activate if mouse is down
                if (pointer.buttons == 1 || pointer.buttons == 2) {
                    UIGlobals.Active = highlightSprite;
                }

                self._UpdateHighlights();
            });

            highlightSprite.on("pointerout", function (pointer, event) {
                if (UIGlobals.Hot == highlightSprite) {
                    UIGlobals.Hot = null;
                }
                

                self._UpdateHighlights();
            });

            highlightSprite.on("pointerdown", function (pointer, localX, localY, event) {
                UIGlobals.Active = highlightSprite;
                UIGlobals.Hot = highlightSprite;
    
                self._UpdateHighlights();
            });
        }
    }

    Layout(x, y, width) {
        this._x = x;
        this._y = y;

        if (width === undefined) {
            if (this._itemTexts.length == 0) {
                this._width = this._defaultWidth;
            }
            else {
                this._width = 0;
                for (const text of this._itemTexts) {
                    if (text != null) {
                        if (text.width > this._width) {
                            this._width = text.width;
                        }
                    }
                }
                if (this._width == 0) {
                    this._width = this._defaultWidth;
                }
                this._width += UIGlobals.Sizes.PopupMenuMargin * 2;
            }
        }
        else {
            this._width = width;
        }

        if (this._itemTexts.length == 0) {
            this._height = 100;
        }
        else {
            this._height = UIGlobals.Sizes.PopupMenuMargin;
            this._height += this._numLabels * UIGlobals.Sizes.PopupMenuItemHeight;
            this._height += this._numSeperators * UIGlobals.Sizes.PopupMenuDividerHeight;
            this._height += this._numSeperators * UIGlobals.Sizes.PopupMenuDividerPadding * 2;
        }

        this._width = Math.ceil(this._width);
        this._height = Math.ceil(this._height);

        const borderSprite = this._borderSprite;
        borderSprite.setTint(UIGlobals.Colors.BorderFraming);
        borderSprite.setPosition(this._x - UIGlobals.Sizes.PopupMenuBorderSize, this._y -  UIGlobals.Sizes.PopupMenuBorderSize);
        borderSprite.setScale(this._width + UIGlobals.Sizes.PopupMenuBorderSize * 2, this._height + UIGlobals.Sizes.PopupMenuBorderSize * 2);

        const backgroundSprite = this._backgroundSprite;
        backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
        backgroundSprite.setPosition(this._x, this._y);
        backgroundSprite.setScale(this._width, this._height);

        x = this._x + UIGlobals.Sizes.PopupMenuMargin;
        y = this._y + UIGlobals.Sizes.PopupMenuMargin / 2;

        const size = this._labels.length;
        for (let i = 0; i < size; ++i) {
            if (this._labels[i] === "") {
                const seperatorSprite = this._seperatorSprites[i];
                seperatorSprite.setPosition(x - UIGlobals.Sizes.PopupMenuMargin / 2, y + UIGlobals.Sizes.PopupMenuDividerPadding);
                seperatorSprite.setScale(this._width - UIGlobals.Sizes.PopupMenuMargin, UIGlobals.Sizes.PopupMenuDividerHeight);
                seperatorSprite.setTint(UIGlobals.Colors.BorderFraming);
                y += UIGlobals.Sizes.PopupMenuDividerHeight + UIGlobals.Sizes.PopupMenuDividerPadding * 2;
            }
            else {
                this._itemTexts[i].setPosition(x, y);

                const highlightSprite = this._highlightSprites[i];
                highlightSprite.setPosition(x - UIGlobals.Sizes.PopupMenuMargin / 2, y);// - UIGlobals.Sizes.PopupMenuMargin / 2 - UIGlobals.Sizes.PopupMenuItemHeight / 2);
                highlightSprite.setScale(this._width - UIGlobals.Sizes.PopupMenuMargin, UIGlobals.Sizes.PopupMenuItemHeight);// + UIGlobals.Sizes.PopupMenuMargin);

                y += UIGlobals.Sizes.PopupMenuItemHeight;
            }
        }
    }

    Show() {
        this.SetVisibility(true);
    }

    Hide() {
        this.SetVisibility(false);
    }

    SetVisibility(visible) {
        const active = visible;
        this._visible = visible;
        this._borderSprite.setActive(active).setVisible(visible);
        this._backgroundSprite.setActive(active).setVisible(visible);
        for (const sprite of this._seperatorSprites) {
            if (sprite != null) {
                sprite.setActive(active).setVisible(visible);
            }
        }
        for (const text of this._itemTexts) {
            if (text != null) {
                text.setActive(active).setVisible(visible);
            }
        }
        for (const sprite of this._highlightSprites) {
            if (sprite != null) {
                sprite.setActive(active).setVisible(visible);
            }
        }

        if (!visible) {
            for (const sprite of this._highlightSprites) {
                if (sprite != null) {
                    if (UIGlobals.Hot == sprite) {
                        UIGlobals.Hot = null;
                    }
                    if (UIGlobals.Active == sprite) {
                        UIGlobals.Active = null;
                    }
                }
            }
        }
        else {
            this._UpdateHighlights();
        }
    }

    _UpdateHighlights() {
        for (const sprite of this._highlightSprites) {
            if (sprite != null) {
                if (UIGlobals.Active == sprite) {
                    sprite.setTint(UIGlobals.Colors.TopMenuButtonActive)
                }
                else if (UIGlobals.Hot == sprite) {
                    sprite.setTint(UIGlobals.Colors.TopMenuButtonHot)
                }
                else {
                    sprite.setTint(UIGlobals.Colors.TopMenuButtonIdle)
                }
            }
        }
    }

    _ContainsHighlight(highlightSprite) {
        for (const sprite of this._highlightSprites) {
            if (sprite != null) {
                if (highlightSprite == sprite) {
                    return true;
                }
            }
        }
        return false;
    }
}
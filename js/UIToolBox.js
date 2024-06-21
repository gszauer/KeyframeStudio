import UIGlobals from './UIGlobals.js'
import UIToolBoxButton from './UIToolboxButton.js'

export default class UIToolBox {
    _scene = null;

    _y = 0;
    _width = 0;
    _height = 0;
    
    _backgroundSprite = null;
    _borderSprite = null;

    _buttons = [];
    _seperators = [];

    _activeIndex = -1;
    _selected = null;
    onToolChanged = null; // onToolChanged(buttonName, buttonObject)

    constructor(scene, _onToolChanged = null) {
        this._scene = scene;
        this.onToolChanged = _onToolChanged;
        const self = this;

        this._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._borderSprite.setDepth(UIGlobals.WidgetLayer);
        this._borderSprite.setOrigin(0, 0);

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);

        scene.input.on("pointerup", function(pointer, currentlyOver) {
            const size = self._buttons.length;
            let reColor = false;
            for (let i = 0; i < size; ++i) {
                if (self._buttons[i] != null) {
                    if (self._buttons[i].HandlePointerUpEvent(pointer, currentlyOver)) {
                        const _iconName = self._buttons[i]._iconName;
                        self._selected = self._GetButtonByName(_iconName);
                        console.log("Selected: " + _iconName);
                        if (self.onToolChanged != null) {
                            self.onToolChanged(_iconName, self._buttons[i]);
                        }
                        reColor = true;
                    }
                }
            }
            if (reColor) {
                for (let i = 0; i < size; ++i) {
                    if (self._buttons[i] != null) {
                        self._buttons[i].UpdateColors();
                    }
                }
            }
        });
    }

    Add(iconName, callback = null) {
        const self = this;

        if (iconName == "" || iconName == null) {
            const seperator = this._scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            this._buttons.push(null);
            this._seperators.push(seperator);
            seperator.setDepth(UIGlobals.WidgetLayer);
            seperator.setScale(UIGlobals.Sizes.ToolboxButtonSize, UIGlobals.Sizes.ToolboxButtonDividerHeight);
            seperator.setOrigin(0.5, 0.5);

            return seperator;
        }

        /*const wrappedCallback = function(_iconName) {
            self._selected = self._GetButtonByName(_iconName);
            console.log("Selected: " + _iconName);
            if (callback != null) {
                callback(_iconName);
            }
            self.Layout();
        };*/

        const button = new UIToolBoxButton(this._scene, iconName, callback);
        button._parent = this;
        this._buttons.push(button);
        this._seperators.push(null);
        return button;
    }

    _GetButtonByName(iconName) {
        const size = this._buttons.length;
        for (let i = 0; i < size; ++i) {
            if (this._buttons[i] != null) {
                if (this._buttons[i]._iconName == iconName) {
                    return this._buttons[i];
                }
            }
        }
        return null;
    }

    Layout() {
        let x = 0;
        let y = this._y = UIGlobals.Sizes.TopMenuHeight + UIGlobals.Sizes.EditorBarHeight;
        let width = this._width = UIGlobals.Sizes.ToolboxWidth;
        let height = this._height = this._scene.scale.height - y;

        this._borderSprite.setPosition(x, y);
        this._borderSprite.setScale(width, height);
        this._borderSprite.setTint(UIGlobals.Colors.BorderDecorative);

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(width - UIGlobals.Sizes.ToolboxPadding, height);
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);

        const size = this._buttons.length;
        x = x + width / 2; // Center
        y += UIGlobals.Sizes.ToolboxButtonSize / 2; // Start at 0
        y += (width - UIGlobals.Sizes.ToolboxButtonSize) / 2; // Pad out equal to button

        for (let i = 0; i < size; ++i) {
            if (this._buttons[i] != null) {
                this._buttons[i].Layout(x, y);
                y += UIGlobals.Sizes.ToolboxButtonSize;
                y += UIGlobals.Sizes.ToolboxButtonPadding;
            }
            else {
                y -= UIGlobals.Sizes.ToolboxButtonSize * 0.5;
                this._seperators[i].setPosition(x, y);// - UIGlobals.Sizes.ToolboxButtonPadding * 0.5);
                this._seperators[i].setTint(UIGlobals.Colors.BorderDecorative);
                y += UIGlobals.Sizes.ToolboxButtonDividerHeight;
                y += UIGlobals.Sizes.ToolboxButtonPadding;
                y += UIGlobals.Sizes.ToolboxButtonSize * 0.5;
            }
        }
    }
}
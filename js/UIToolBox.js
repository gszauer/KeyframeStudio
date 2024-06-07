import UIGlobals from './UIGlobals.js'
import UIToolBoxButton from './UIToolBoxButton.js'

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

    constructor(scene) {
        this._scene = scene;

        this._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._borderSprite.setDepth(UIGlobals.WidgetLayer);
        this._borderSprite.setOrigin(0, 0);

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);
    }

    Add(iconName, callback) {
        if (iconName == "" || iconName == null) {
            const seperator = this._scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            this._buttons.push(null);
            this._seperators.push(seperator);
            seperator.setDepth(UIGlobals.WidgetLayer);
            seperator.setScale(UIGlobals.Sizes.ToolboxButtonSize, UIGlobals.Sizes.ToolboxButtonDividerHeight);
            seperator.setOrigin(0.5, 0.5);

            return seperator;
        }

        const button = new UIToolBoxButton(this._scene, iconName, callback);
        this._buttons.push(button);
        this._seperators.push(null);
        return button;
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
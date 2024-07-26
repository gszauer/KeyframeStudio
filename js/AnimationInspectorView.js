import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'
import UIColorButton from './UIColorButton.js'
import ColorRGB from './ColorRGB.js'
import UIToggle from './UIToggle.js'

export default class AnimationInspectorView extends UIView {
    _focused = null;

    _backgroundSprite = null;

    _transformLabel = null;
    
    _nameLabel = null;
    _nameTextField = null;

    _frameRateLabel = null;
    _frameRateTextField = null;

    _frameCountLabel = null;
    _frameCountTextField = null;

    _loopingCheckBox = null;

    _animView = null;

    constructor(scene, parent = null, animView = null) {
        super(scene, parent);
        this._animView = animView;

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);

        this._transformLabel = scene.add.bitmapText(0, 0, UIGlobals.Font400, name);
        this._transformLabel.setDepth(UIGlobals.WidgetLayer);
        this._transformLabel.text = "Animation";

        this._nameLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._nameLabel.setDepth(UIGlobals.WidgetLayer);
        this._nameLabel.text = "Name";

        this._frameCountLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._frameCountLabel.setDepth(UIGlobals.WidgetLayer);
        this._frameCountLabel.text = "Frame Count";

        this._frameRateLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._frameRateLabel.setDepth(UIGlobals.WidgetLayer);
        this._frameRateLabel.text = "Frame Rate (FPS)";

        const self = this;

        const NumerisizeString = (str) => {
            let result = "";
            if (str[0] == '-') {
                result += '-';
            }
            let period = false;
            for (let i = 0, length = str.length; i < length; ++i) {
                if (str[i] >= '0' && str[i] <= '9') {
                    result += str[i];
                }
                if (str[i] == '.' && !period) {
                    result += '.';
                    period = true;
                }
            }
            if (result == "") {
                result = '0';
            }
            return result;
        }

        this._nameTextField = new UITextBox(scene, "");
        this._nameTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                self._focused.name = "" + value;
                self._animView.UpdateNames();
            }
        };
        this._nameTextField.Disable();

        this._frameCountTextField = new UITextBox(scene, "");
        this._frameCountTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = NumerisizeString(value);
                self._focused.frameCount = Number(value);
                self._frameCountTextField.text = "" + value;
            }
        }
        this._frameCountTextField.Disable();

        this._frameRateTextField = new UITextBox(scene, "");
        this._frameRateTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = NumerisizeString(value);
                self._focused.frameRate = Number(value);
                self._frameRateTextField.text = "" + value;
            }
        }
        this._frameRateTextField.Disable();

        this._loopingCheckBox = new UIToggle(scene, "Looping", (value, control) => {
            self._focused.looping = value;
        });
        this._loopingCheckBox.Disable();

        animView.onSelectionChanged = (animTarget) => {
            self.FocusOn(animTarget);
        };
   }

    UpdateColors() {
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
        this._loopingCheckBox.UpdateColors();
        this._frameRateTextField.UpdateColors();
        this._frameCountTextField.UpdateColors();

        let borderTint = UIGlobals.Colors.ElementBorderTintIdle;
        if (!this._loopingCheckBox._enabled) {
            borderTint = UIGlobals.Colors.IconDisabled;
        }

        this._nameLabel.setTint(borderTint);
        this._frameRateLabel.setTint(borderTint);
        this._frameCountLabel.setTint(borderTint);
    }

    Layout(x, y, width, height) {
        if (x === undefined) { x = this._x; }
        if (y === undefined) { y = this._y; }
        if (width === undefined) { width = this._width; }
        if (height === undefined) { height = this._height; }

        super.Layout(x, y, width, height);
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        
        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(width, height);

        const margin = UIGlobals.Sizes.InspectorTitleMargin;
        const skip = UIGlobals.Sizes.InspectorTitleSkip;

        x += margin;

        this._transformLabel.setPosition(x, y);

        x += margin;
        y = y + this._transformLabel.height + skip;

        this._nameLabel.setPosition(x, y);
        y = y + this._nameLabel.height + skip;

        this._nameTextField.Layout(x, y, width - margin * 4);

        y += this._nameTextField._height;
        y += skip;

        let rowWidth = Math.floor((width - margin * 6) / 2);

        this._frameRateLabel.setPosition(x, y);
        this._frameCountLabel.setPosition(x + rowWidth + margin * 2, y);

        y = y + this._frameRateLabel.height + skip;
        this._frameRateTextField.Layout(x, y, rowWidth);
        this._frameCountTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);

        y += this._frameRateTextField._height + skip * 2.5;
        this._loopingCheckBox.Layout(x, y)
    }

    _SetVisibility(visible) {
        this._nameTextField.SetVisibility(visible);
        this._frameCountTextField.SetVisibility(visible);
        this._frameRateTextField.SetVisibility(visible);
        this._loopingCheckBox.SetVisibility(visible);
        this._backgroundSprite.setActive(visible).setVisible(visible);
        this._transformLabel.setActive(visible).setVisible(visible);
        this._nameLabel.setActive(visible).setVisible(visible);
        this._frameCountLabel.setActive(visible).setVisible(visible);
        this._frameRateLabel.setActive(visible).setVisible(visible);
    }

    Hide() {
        this._SetVisibility(false);
    }

    Show() {
        this._SetVisibility(true);
    }

    FocusOn(animation) {
        this._focused = animation;

        let name = "";
        let frameRate = "";
        let frameCount = "";
        let looping = false;

        if (animation != null) {
            name = "" + animation.name;
            frameRate = animation.frameRate;
            frameCount = animation.frameCount;
            looping = animation.looping;
        }


        this._nameTextField.text = name;
        this._frameRateTextField.text = "" + frameRate;
        this._frameCountTextField.text = "" + frameCount;
        this._loopingCheckBox._state = looping;

        if (animation != null) {
            this._nameTextField.Enable();
            this._frameRateTextField.Enable();
            this._frameCountTextField.Enable();
            this._loopingCheckBox.Enable();
        }
        else {
            this._nameTextField.Disable();
            this._frameRateTextField.Disable();
            this._frameCountTextField.Disable();
            this._loopingCheckBox.Disable();
        }

        this.UpdateColors();
    }
}
import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITextBox from './UITextBox.js'


export default class AssetsView extends UIView {
    _filePathLabel = null;
    _filePathTextField = null;
    _backgroundSprite = null;

    constructor(scene, parent = null) {
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

        super(scene, parent);
        const self = this;

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);

        this._filePathLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._filePathLabel.setDepth(UIGlobals.WidgetLayer);
        this._filePathLabel.text = "Sprite Sheet File";

        this._filePathTextField = new UITextBox(scene, "");
        this._filePathTextField.onTextEdit = (value) => {
            /*if (self._focused != null) {
                self._focused.name = value;
                self._focused._userData.drawOrder._labelText.text = value;
            }*/
        };
    }

    UpdateColors() {
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(width, height);

        const margin = UIGlobals.Sizes.InspectorTitleMargin;
        const skip = UIGlobals.Sizes.InspectorTitleSkip;

        x += margin;
        y += margin;

        this._filePathLabel.setPosition(x, y);
        y = y + this._filePathLabel.height + skip;

        this._filePathTextField.Layout(x, y, width - margin * 2);
        y += this._filePathTextField._height + skip;

    }

    SetVisibility(value) {
        this._backgroundSprite.setActive(value).setVisible(value);
        this._filePathTextField.SetVisibility(value);
        this._filePathLabel.setActive(value).setVisible(value);
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
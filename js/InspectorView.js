import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'
import UIColorButton from './UIColorButton.js'
import ColorRGB from './ColorRGB.js'
import UIToggle from './UIToggle.js'

export default class InspectorView extends UIView {
    _focused = null;

    _backgroundSprite = null;

    _transformLabel = null;
    _spriteLabel = null;
    
    _nameLabel = null;
    _nameTextField = null;

    _positionXLabel = null;
    _positionXTextField = null;

    _positionYLabel = null;
    _positionYTextField = null;

    _rotationLabel = null;
    _rotationTextField = null;

    _scaleXLabel = null;
    _scaleXTextField = null;

    _scaleYLabel = null;
    _scaleYTextField = null;

    _spriteSheetLabel = null;
    _spriteSheetDropdown = null;

    _tintLabel = null;
    _tintButton = null;

    _frameXLabel = null;
    _frameXTextField = null;

    _frameYLabel = null;
    _frameYTextField = null;

    _visibleLabel = null;
    _visibleDropdown = null;

    _frameWLabel = null;
    _frameWTextField = null;

    _frameHLabel = null;
    _frameHTextField = null;

    _pivotXLabel = null;
    _pivotXTextField = null;

    _pivotYLabel = null;
    _pivotYTextField = null;

    _spriteIsEnabledCheckBox = null;

    _hierarchyView = null;
    _sceneView = null;
    _assetsView = null;

    get texture() {
        return this._assetsView.atlasTexture;
    }

    constructor(scene, parent = null) {
        super(scene, parent);

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);

        this._spriteIsEnabledCheckBox = new UIToggle(scene, "", null);

        this._transformLabel = scene.add.bitmapText(0, 0, UIGlobals.Font400, name);
        this._transformLabel.setDepth(UIGlobals.WidgetLayer);
        this._transformLabel.text = "Transform";

        this._spriteLabel = scene.add.bitmapText(0, 0, UIGlobals.Font400, name);
        this._spriteLabel.setDepth(UIGlobals.WidgetLayer);
        this._spriteLabel.text = "Sprite";

        this._nameLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._nameLabel.setDepth(UIGlobals.WidgetLayer);
        this._nameLabel.text = "Name";

        this._positionXLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._positionXLabel.setDepth(UIGlobals.WidgetLayer);
        this._positionXLabel.text = "X Position";

        this._positionYLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._positionYLabel.setDepth(UIGlobals.WidgetLayer);
        this._positionYLabel.text = "Y Position";

        this._rotationLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._rotationLabel.setDepth(UIGlobals.WidgetLayer);
        this._rotationLabel.text = "Rotation";

        this._scaleXLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._scaleXLabel.setDepth(UIGlobals.WidgetLayer);
        this._scaleXLabel.text = "X Scale";

        this._scaleYLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._scaleYLabel.setDepth(UIGlobals.WidgetLayer);
        this._scaleYLabel.text = "Y Scale";

        this._spriteSheetLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._spriteSheetLabel.setDepth(UIGlobals.WidgetLayer);
        this._spriteSheetLabel.text = "Sprite Atlas (json presets)";

        this._tintLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._tintLabel.setDepth(UIGlobals.WidgetLayer);
        this._tintLabel.text = "Tint";

        this._frameXLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._frameXLabel.setDepth(UIGlobals.WidgetLayer);
        this._frameXLabel.text = "Frame X";

        this._frameYLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._frameYLabel.setDepth(UIGlobals.WidgetLayer);
        this._frameYLabel.text = "Frame Y";

        this._visibleLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._visibleLabel.setDepth(UIGlobals.WidgetLayer);
        this._visibleLabel.text = "Visible";

        this._frameWLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._frameWLabel.setDepth(UIGlobals.WidgetLayer);
        this._frameWLabel.text = "Width";

        this._frameHLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._frameHLabel.setDepth(UIGlobals.WidgetLayer);
        this._frameHLabel.text = "Height";

        this._pivotXLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._pivotXLabel.setDepth(UIGlobals.WidgetLayer);
        this._pivotXLabel.text = "Pivot X";

        this._pivotYLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._pivotYLabel.setDepth(UIGlobals.WidgetLayer);
        this._pivotYLabel.text = "Pivot Y";

        const self = this;

        let popup = new UIPopup(scene);
        popup.Add("True", () => {
            if (self._focused != null) {
                self._focused._userData.sprite.SetVisibility(true);
                self._visibleDropdown.selected = "True";
            }
        });
        popup.Add("False", () => {
            if (self._focused != null) {
                self._focused._userData.sprite.SetVisibility(false);
                self._visibleDropdown.selected = "False";
            }
        });
        this._visibleDropdown = new UIDropdown(scene, popup);

        popup = new UIPopup(scene);
        popup.Add("None");
        popup.Add("None");
        this._spriteSheetDropdown = new UIDropdown(scene, popup);

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
                self._focused.name = value;
                self._focused._userData.drawOrder._labelText.text = value;
            }
        };
        this._nameTextField.Disable();

        this._positionXTextField = new UITextBox(scene, "");
        this._positionXTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = NumerisizeString(value);
                self._focused._userData.transform.x = Number(value);
                self._positionXTextField.text = "" + value;
                self._hierarchyView._UpdateTransforms();
            }
        }
        this._positionXTextField.Disable();

        this._positionYTextField = new UITextBox(scene, "");
        this._positionYTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = NumerisizeString(value);
                self._focused._userData.transform.y = Number(value);
                self._positionYTextField.text = "" + value;
                self._hierarchyView._UpdateTransforms();
            }
        }
        this._positionYTextField.Disable();

        this._rotationTextField = new UITextBox(scene, "");
        this._rotationTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = NumerisizeString(value);
                while(value < 0) {  value += 360; }
                while (value > 360) { value -= 360; }
                if (value == 360) { value = 0; }
                self._focused._userData.transform.degrees = Number(value);
                this._rotationTextField.text = "" + value;
                self._hierarchyView._UpdateTransforms();
            }
        }
        this._rotationTextField.Disable();

        this._scaleXTextField = new UITextBox(scene, "");
        this._scaleXTextField.Disable();
        this._scaleYTextField = new UITextBox(scene, "");
        this._scaleYTextField.Disable();

        this._scaleXTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = NumerisizeString(value);
                self._focused._userData.transform.scaleX = Number(value);
                self._scaleXTextField.text = "" + value;
                self._hierarchyView._UpdateTransforms();
            }
        }
        this._scaleYTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = NumerisizeString(value);
                self._focused._userData.transform.scaleY = Number(value);
                self._scaleYTextField.text = "" + value;
                self._hierarchyView._UpdateTransforms();
            }
        }
        this._frameXTextField = new UITextBox(scene, "");
        this._frameYTextField = new UITextBox(scene, "");
        this._frameWTextField = new UITextBox(scene, "");
        this._frameHTextField = new UITextBox(scene, "");
        this._pivotXTextField = new UITextBox(scene, "");
        this._pivotYTextField = new UITextBox(scene, "");
        this._tintButton = new UIColorButton(scene);

        const UpdateFrame = () => {
            this._assetsView.UpdateFrames();
        };

        this._tintButton.onColorChanged = (rgb) => {
            if (self._focused != null) {
                self._focused._userData.sprite.color = rgb;
                self._tintButton.color = rgb;
                UpdateFrame();
            }
        };

        this._frameXTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = Number(NumerisizeString(value));
                self._focused._userData.sprite.x = value;
                self._frameXTextField.text = "" + value;
                UpdateFrame();
            }
        };


        this._frameYTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = Number(NumerisizeString(value));
                self._focused._userData.sprite.y = value;
                self._frameYTextField.text = "" + value;
                UpdateFrame();
            }
        };
        this._frameWTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = Number(NumerisizeString(value));
                self._focused._userData.sprite.width = value;
                self._frameWTextField.text = "" + value;
                UpdateFrame();
            }
        };
        this._frameHTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = Number(NumerisizeString(value));
                self._focused._userData.sprite.height = value;
                self._frameHTextField.text = "" + value;
                UpdateFrame();
            }
        };
        this._pivotXTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = Number(NumerisizeString(value));
                self._focused._userData.sprite.pivotX = value;
                self._pivotXTextField.text = "" + value;
                UpdateFrame();
            }
        };
        this._pivotYTextField.onTextEdit = (value) => {
            if (self._focused != null) {
                value = Number(NumerisizeString(value));
                self._focused._userData.sprite.pivotY = value;
                self._pivotYTextField.text = "" + value;
                UpdateFrame();
            }
        };
        this._spriteIsEnabledCheckBox.onToggle = (val, toggle) => {
            if (self._focused != null) {
                if (val) {
                    self._focused._userData.sprite.Enable();
                }
                else {
                    self._focused._userData.sprite.Disable();
                }
            }
        };

        this._frameXTextField.Disable();
        this._frameYTextField.Disable();
        this._frameWTextField.Disable();
        this._spriteIsEnabledCheckBox.Disable();
        this._frameHTextField.Disable();
        this._pivotXTextField.Disable();
        this._pivotYTextField.Disable();
        this._spriteSheetDropdown.Disable();
        this._visibleDropdown.Disable();
        this._tintButton.Disable();
   }

   UpdatePresetPopup(newPopup) {
    this._spriteSheetDropdown.SetMenu(newPopup);
   }

    UpdateColors() {
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
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

        const rowWidth = (width - margin * 8) / 3 ;

        this._rotationLabel.setPosition(x, y);
        this._positionXLabel.setPosition(x + rowWidth + margin * 2, y);
        this._positionYLabel.setPosition(x + rowWidth * 2 + margin * 4, y);

        y = y + this._rotationLabel.height + skip;
        this._rotationTextField.Layout(x, y, rowWidth);
        this._positionXTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._positionYTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y += this._rotationTextField._height +skip;
        this._scaleXLabel.setPosition(x + rowWidth + margin * 2, y);
        this._scaleYLabel.setPosition(x + rowWidth * 2 + margin * 4, y);

        y = y + this._scaleYLabel.height + skip;
        this._scaleXTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._scaleYTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y = y + this._scaleYTextField._height + skip;
        y += Math.floor(margin * 0.5);

        this._spriteIsEnabledCheckBox.Layout(x - margin, y + 7);
        this._spriteLabel.setPosition(x - (margin / 2) + UIGlobals.Sizes.CheckboxSize, y);
        y = y + this._spriteLabel.height + Math.floor(skip * 2);

        const frameMap = this._assetsView.frameMap;
        if (frameMap != null && frameMap.size > 0) {
            this._spriteSheetLabel.setPosition(x, y);
            y = y + this._spriteSheetLabel.height + skip;
            
            this._spriteSheetDropdown.Layout(x, y, width - margin * 4);
            y = y + this._spriteSheetDropdown._height + skip;
        }
        else {
            this._spriteSheetLabel.setActive(false).setVisible(false);
            this._spriteSheetDropdown.Disable();
        }

        this._visibleLabel.setPosition(x, y);
        this._frameXLabel.setPosition(x + rowWidth + margin * 2, y, rowWidth);
        this._frameYLabel.setPosition(x + rowWidth * 2 + margin * 4, y, rowWidth);
        y = y + this._frameYLabel.height + skip;
        
        this._visibleDropdown.Layout(x, y, rowWidth);
        this._frameXTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._frameYTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);
        y = y + this._frameYTextField._height + skip;
        
        this._tintLabel.setPosition(x, y);
        this._frameWLabel.setPosition(x + rowWidth + margin * 2, y, rowWidth);
        this._frameHLabel.setPosition(x + rowWidth * 2 + margin * 4, y, rowWidth);
        y = y + this._frameHLabel.height + skip;

        this._tintButton.Layout(x, y, rowWidth, UIGlobals.Sizes.TextboxHeight);
        this._frameWTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._frameHTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);
        y = y + this._frameHTextField._height + skip;

        
        this._pivotXLabel.setPosition(x + rowWidth + margin * 2, y, rowWidth);
        this._pivotYLabel.setPosition(x + rowWidth * 2 + margin * 4, y, rowWidth);
        y = y + this._pivotYLabel.height + skip;
        
        this._pivotXTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._pivotYTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);
    }

    _SetVisibility(visible) {
        this._nameTextField.SetVisibility(visible);
        this._positionXTextField.SetVisibility(visible);
        this._positionYTextField.SetVisibility(visible);
        this._rotationTextField.SetVisibility(visible);
        this._scaleXTextField.SetVisibility(visible);
        this._scaleYTextField.SetVisibility(visible);
        this._frameXTextField.SetVisibility(visible);
        this._frameYTextField.SetVisibility(visible);
        this._frameWTextField.SetVisibility(visible);
        this._frameHTextField.SetVisibility(visible);
        //this._alphaTextField.SetVisibility(visible);
        this._pivotXTextField.SetVisibility(visible);
        this._pivotYTextField.SetVisibility(visible);
        this._spriteSheetDropdown.SetVisibility(visible);
        this._tintButton.SetVisibility(visible);
        this._visibleDropdown.SetVisibility(visible);
        this._backgroundSprite.setActive(visible).setVisible(visible);
        this._transformLabel.setActive(visible).setVisible(visible);
        this._spriteIsEnabledCheckBox.SetVisibility(visible);
        this._spriteLabel.setActive(visible).setVisible(visible);
        this._nameLabel.setActive(visible).setVisible(visible);
        this._positionXLabel.setActive(visible).setVisible(visible);
        this._positionYLabel.setActive(visible).setVisible(visible);
        this._rotationLabel.setActive(visible).setVisible(visible);
        this._scaleXLabel.setActive(visible).setVisible(visible);
        this._scaleYLabel.setActive(visible).setVisible(visible);
        //this._scaleModeDropdown.SetVisibility(visible);
        //this._scaleModeLabel.setActive(visible).setVisible(visible);
        this._spriteSheetLabel.setActive(visible).setVisible(visible);
        this._tintLabel.setActive(visible).setVisible(visible);
        this._frameXLabel.setActive(visible).setVisible(visible);
        this._frameYLabel.setActive(visible).setVisible(visible);
        this._visibleLabel.setActive(visible).setVisible(visible);
        this._frameWLabel.setActive(visible).setVisible(visible);
        this._frameHLabel.setActive(visible).setVisible(visible);
        //this._alphaLabel.setActive(visible).setVisible(visible);
        this._pivotXLabel.setActive(visible).setVisible(visible);
        this._pivotYLabel.setActive(visible).setVisible(visible);
    }

    Hide() {
        this._SetVisibility(false);
    }

    Show() {
        this._SetVisibility(true);
    }

    FocusOn(node) {
        /*if (node === undefined) {
            node = this._focused;
        }*/

        this._focused = node;

        let name = "";
        let xPos = "";
        let yPos = "";
        let xScale = "";
        let yScale = "";
        let rotation = "";

        let frameX = "";
        let frameY = "";
        let frameW = "";
        let frameH = "";
        let pivotX = "";
        let pivotY = "";
        //let alpha = "";
        let visible = "True";
        let drawIndex = "";
        let color = new ColorRGB(1, 1, 1);
        let spriteEnabled = false;

        if (node != null) {
            if (node._userData == null) {
                throw new Error("User data can't be null here!");
            }
            const xForm = node._userData.transform;
            const sprite = node._userData.sprite;

            name = node._name;
            xPos = "" + xForm.x;
            yPos = "" + xForm.y;
            xScale = "" + xForm.scaleX;
            yScale = "" + xForm.scaleY;
            rotation = "" + xForm.degrees;

            frameX = "" + sprite.x;
            frameY = "" + sprite.y;
            frameW = "" + sprite.width;
            frameH = "" + sprite.height;
            pivotX = "" + sprite.pivotX;
            pivotY = "" + sprite.pivotY;
            //alpha = "" + sprite.alpha;
            visible = sprite.visible? "True" : "False";
            color = sprite.color;
            spriteEnabled = sprite.enabled;

            drawIndex = "Draw index: " + sprite.sprite.depth;
        }
       

        this._nameTextField.text = name;
        this._positionXTextField.text = xPos;
        this._positionYTextField.text = yPos;
        this._rotationTextField.text = rotation;
        this._scaleXTextField.text = xScale;
        this._scaleYTextField.text = yScale;

        //this._alphaTextField.text = alpha;
        this._pivotYTextField.text = pivotX;
        this._pivotXTextField.text = pivotY;
        this._frameXTextField.text = frameX;
        this._frameYTextField.text = frameY;
        this._frameWTextField.text = frameW;
        this._frameHTextField.text = frameH;
        this._visibleDropdown.selected = visible;
        this._tintButton.color = color;
        this._spriteIsEnabledCheckBox._state = spriteEnabled;

        if (node != null) {
            this._nameTextField.Enable();
            this._positionXTextField.Enable();
            this._positionYTextField.Enable();
            this._rotationTextField.Enable();
            this._scaleXTextField.Enable();
            this._scaleYTextField.Enable();
            this._frameXTextField.Enable();
            this._frameYTextField.Enable();
            this._frameWTextField.Enable();
            this._spriteIsEnabledCheckBox.Enable();
            this._frameHTextField.Enable();
            this._pivotXTextField.Enable();
            this._pivotYTextField.Enable();
            this._visibleDropdown.Enable();
            this._tintButton.Enable();

            const frameMap = this._assetsView.frameMap;
            if (frameMap != null && frameMap.size > 0) {
                this._spriteSheetDropdown.Enable();
                this._spriteSheetLabel.setActive(true).setVisible(true);
            }
            else {
                this._spriteSheetDropdown.Disable();
                this._spriteSheetLabel.setActive(false).setVisible(false);
            }
        }
        else {
            this._nameTextField.Disable();
            this._positionXTextField.Disable();
            this._positionYTextField.Disable();
            this._rotationTextField.Disable();
            this._scaleXTextField.Disable();
            this._scaleYTextField.Disable();
            this._frameXTextField.Disable();
            this._frameYTextField.Disable();
            this._frameWTextField.Disable();
            this._spriteIsEnabledCheckBox.Disable();
            this._frameHTextField.Disable();
            this._pivotXTextField.Disable();
            this._pivotYTextField.Disable();
            this._spriteSheetDropdown.Disable();
            this._visibleDropdown.Disable();
            this._tintButton.Disable();
        }
    }
}
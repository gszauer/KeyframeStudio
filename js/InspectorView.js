import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'

export default class InspectorView extends UIView {
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

    _scaleModeLabel = null;
    _scaleModeDropdown = null;

    _spriteSheetLabel = null;
    _spriteSheetDropdown = null;

    _tintLabel = null; // Tint | Alpha | Visible
    _tintColorPicker = null;

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

    _alphaLabel = null;
    _alphaTextField = null;

    _pivotXLabel = null;
    _pivotXTextField = null;

    _pivotYLabel = null;
    _pivotYTextField = null;

    constructor(scene, parent = null) {
        super(scene, parent);

        var colorPicker = scene.rexUI.add.colorPicker({
            // x: 0,
            // y: 0,
            // anchor: undefined,
            // width: undefined,
            // height: undefined,
            // origin: 0.5
            // originX:
            // originY:
        
            background: backgroundGameObject,
        
            hPalette: {
                position: 'bottom',
                size: 10,
                width: undefined,
                height: undefined,       
            },
        
            svPalette: {
                width: undefined,
                height: undefined,
            },
        
            valuechangeCallback: function(newValue, oldValue, knob) {
            },
            valuechangeCallbackScope: undefined,
        
            value: 0xffffff,
        
            // space: { left: 0, right:0, top:0, bottom:0, item:0 },
        
            // name: '',
            // draggable: false,
            // sizerEvents: false,
            // enableLayer: false,    
        });

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);

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

        this._scaleModeLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._scaleModeLabel.setDepth(UIGlobals.WidgetLayer);
        this._scaleModeLabel.text = "Uniform";

        this._spriteSheetLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._spriteSheetLabel.setDepth(UIGlobals.WidgetLayer);
        this._spriteSheetLabel.text = "Texture Atlas";

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

        this._alphaLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._alphaLabel.setDepth(UIGlobals.WidgetLayer);
        this._alphaLabel.text = "Alpha";

        this._pivotXLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._pivotXLabel.setDepth(UIGlobals.WidgetLayer);
        this._pivotXLabel.text = "Pivot X";

        this._pivotYLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._pivotYLabel.setDepth(UIGlobals.WidgetLayer);
        this._pivotYLabel.text = "Pivot Y";

        let popup = new UIPopup(scene);
        popup.Add("False");
        popup.Add("True");
        this._scaleModeDropdown = new UIDropdown(scene, popup);

        popup = new UIPopup(scene);
        popup.Add("True");
        popup.Add("False");
        this._visibleDropdown = new UIDropdown(scene, popup);

        popup = new UIPopup(scene);
        popup.Add("None");
        popup.Add("Filler 1 delete me.png");
        popup.Add("Filler 2 delete me.png");
        popup.Add("Filler 3 delete me.png");
        this._spriteSheetDropdown = new UIDropdown(scene, popup);

        this._nameTextField = new UITextBox(scene, "Name");
        this._positionXTextField = new UITextBox(scene, "0");
        this._positionYTextField = new UITextBox(scene, "0");
        this._rotationTextField = new UITextBox(scene, "0");
        this._scaleXTextField = new UITextBox(scene, "1");
        this._scaleYTextField = new UITextBox(scene, "1");
        this._frameXTextField = new UITextBox(scene, "0");
        this._frameYTextField = new UITextBox(scene, "0");
        this._frameWTextField = new UITextBox(scene, "0");
        this._frameHTextField = new UITextBox(scene, "0");
        this._alphaTextField = new UITextBox(scene, "1");
        this._pivotXTextField = new UITextBox(scene, "0");
        this._pivotYTextField = new UITextBox(scene, "0");
   }

    UpdateColors() {
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
    }

    Layout(x, y, width, height) {
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

        this._positionXLabel.setPosition(x, y);
        this._positionYLabel.setPosition(x + rowWidth + margin * 2, y);
        this._rotationLabel.setPosition(x + rowWidth * 2 + margin * 4, y);

        y = y + this._rotationLabel.height + skip;
        this._positionXTextField.Layout(x, y, rowWidth);
        this._positionYTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._rotationTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y += this._rotationTextField._height +skip;
        this._scaleXLabel.setPosition(x, y);
        this._scaleYLabel.setPosition(x + rowWidth + margin * 2, y);
        this._scaleModeLabel.setPosition(x + rowWidth * 2 + margin * 4, y);

        y = y + this._scaleModeLabel.height + skip;
        this._scaleXTextField.Layout(x, y, rowWidth);
        this._scaleYTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._scaleModeDropdown.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y = y + this._scaleYTextField._height + skip;
        y += margin * 0.5;

        this._spriteLabel.setPosition(x - margin, y);
        y = y + this._spriteLabel.height + skip;

        this._spriteSheetLabel.setPosition(x, y);
        y = y + this._spriteSheetLabel.height + skip;

        this._spriteSheetDropdown.Layout(x, y, width - margin * 4);
        y = y + this._spriteSheetDropdown._height + skip;

        this._tintLabel.setPosition(x, y);
        this._frameXLabel.setPosition(x + rowWidth + margin * 2, y, rowWidth);
        this._frameYLabel.setPosition(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y = y + this._frameYLabel.height + skip;
        //this._scaleXTextField.Layout(x, y, rowWidth);
        this._frameXTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._frameYTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y = y + this._frameYTextField._height + skip;
        this._visibleLabel.setPosition(x, y, rowWidth);
        this._frameWLabel.setPosition(x + rowWidth + margin * 2, y, rowWidth);
        this._frameHLabel.setPosition(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y = y + this._frameHLabel.height + skip;
        this._visibleDropdown.Layout(x, y, rowWidth);
        this._frameWTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._frameHTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y = y + this._frameHTextField._height + skip;
        this._alphaLabel.setPosition(x, y, rowWidth);
        this._pivotXLabel.setPosition(x + rowWidth + margin * 2, y, rowWidth);
        this._pivotYLabel.setPosition(x + rowWidth * 2 + margin * 4, y, rowWidth);

        y = y + this._pivotYLabel.height + skip;
        this._alphaTextField.Layout(x, y, rowWidth);
        this._pivotXTextField.Layout(x + rowWidth + margin * 2, y, rowWidth);
        this._pivotYTextField.Layout(x + rowWidth * 2 + margin * 4, y, rowWidth);
    }

    _SetVisibility(visible) {
        // TODO
    }

    Hide() {
        this._SetVisibility(false);
    }

    Show() {
        this._SetVisibility(true);
    }
}
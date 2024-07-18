import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UIToolBarShelf  from './UIToolBarShelf.js';
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'
import UIToggle from './UIToggle.js'
import XForm from './Transform.js'

export default class RotateShelf extends UIToolBarShelf {
    _sceneView = null;

    _snapLabel = null;
    _snapCheckbox = null;
    _snapTextField = null;
    _snapStepSize = 10;
    _snapEnabled = false;

    _circle = null;
    _arc = null;
    _start = null;
    _current = null;

    _radius = 100;
    _mouseDownPos = {x: 0, y: 0};
    _normalizedMouseDownPos = {x: 0, y: 0};
    _dragStartAngle = 0;
    _dragIndicatorStartAngle = 0;
    _dragDeltaAngle = 0;
    _isDragging = false;

    constructor(scene, sceneView) {
        super(scene);
        const self = this;
        this._sceneView = sceneView;

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

        this._snapLabel = scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        this._snapLabel.setDepth(UIGlobals.WidgetLayer);
        this._snapLabel.text = "Snap:";

        this._snapCheckbox = new UIToggle(scene, "", (valueBol, uiToggleObject) => {
            self._snapEnabled = valueBol;
            const val = self._snapTextField.text;
            self._snapStepSize = Number(NumerisizeString(val));
            self.UpdateColors(); 
        });

        this._snapTextField = new UITextBox(scene, "" + this._snapStepSize);
        this._snapTextField.onTextEdit = (value) => {
            self._snapStepSize = Number(NumerisizeString(value));
            self.UpdateColors();
        };
        this._snapTextField._disabled = !this._snapEnabled;

        const circleLineWidth = 8;

        this._start = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._start.setOrigin(0, 0.5);
        self._start.setScale(this._radius, 4);
        self._start.setPosition(0, 0);
        self._start.setDepth(UIGlobals.WidgetLayer);
        self._start.setTint(0x0000ff);
        this._start.setMask(sceneView.mask);

        this._current = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._current.setOrigin(0, 0.5);
        self._current.setPosition(0, 0);
        self._current.setScale(this._radius, 4);
        self._current.setDepth(UIGlobals.WidgetLayer);
        self._current.setTint(0x0000ff);
        this._current.setMask(sceneView.mask);

        this._circle = scene.add.graphics();
        this.UpdateGizmoColor();
        this._circle.setDepth(UIGlobals.OverlayLayer - 5);
        this._circle.setMask(sceneView.mask);

        this._arc = scene.add.graphics();
        this._arc.setDepth(UIGlobals.OverlayLayer - 5);
        this._arc.setMask(sceneView.mask);

        this._circle.setInteractive({ 
            draggable: true,
            hitArea: new Phaser.Geom.Circle(0, 0, this._radius + circleLineWidth), 
            hitAreaCallback: Phaser.Geom.Circle.Contains });
        scene.input.setDraggable(this._circle);

        scene.input.on('dragstart', (pointer, gameObject) => {
            if (gameObject === this._circle) {
                self.DragStart(gameObject, pointer);
                self.UpdateGizmoColor(0xffff00);
            }
        });
        scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
            if (gameObject === this._circle) {
                self.Drag(gameObject, pointer, dragX, dragY);
            }
        });
        scene.input.on('dragend', (pointer, gameObject) => {
            if (gameObject == this._circle) {
                self.DragEnd(gameObject, pointer);
                self.UpdateGizmoColor(0x0000ff);
            }
        });
    }

    get transform() {
        const hierarchy = this._sceneView._hierarchyView;
        if (hierarchy === null || hierarchy === undefined) {
            return null;
        }

        const selected = hierarchy._tree.selected;
        if (selected === null || selected === undefined) {
            return null;
        }

        const userData = selected._userData;
        if (userData === null || userData === undefined) {
            return null;
        }

        const xForm = userData.transform;
        if (xForm === null || xForm === undefined) {
            return null;
        }

        return xForm;
    }

    UpdateGizmoColor(newColor = 0x0000ff) {
        this._circle.clear();
        this._circle.lineStyle(8, newColor, 1);
        this._circle.beginPath();
        this._circle.arc(0, 0, this._radius , 0, Phaser.Math.PI2, true);
        this._circle.strokePath();
        this._current.setTint(newColor);
    }

    SetVisibility(value) {
        super.SetVisibility(value);

        this._snapLabel.setActive(value).setVisible(value);
        this._snapCheckbox.SetVisibility(value);
        this._snapTextField.SetVisibility(value);

        if (this.transform == null) {
            value = false;
        }

        this._circle.setActive(value).setVisible(value);
        this._arc.setActive(value && this._isDragging).setVisible(value && this._isDragging);
        this._start.setActive(value && this._isDragging).setVisible(value && this._isDragging);
        this._current.setActive(value && this._isDragging).setVisible(value && this._isDragging);
    }

    GetViewTransform() {
        const ui = {
            x: UIGlobals.Sizes.ToolboxWidth, y: UIGlobals.Sizes.EditorBarHeight,
            rotation: 0,
            scaleX: 1, scaleY: 1
        };
        const camera = this._sceneView._cameraTransform;
        const view = XForm.Mul(ui, camera, null);
        return view;
    }

    _UpdateTransformPosition() {
        const ui = {
            x: UIGlobals.Sizes.ToolboxWidth, y: UIGlobals.Sizes.EditorBarHeight,
            rotation: 0,
            scaleX: 1, scaleY: 1
        };
        const camera = this._sceneView._cameraTransform;
        const view = XForm.Mul(ui, camera, null);

        const xform = this.transform;
        if (xform) {
            xform.ApplyTransform(this._circle, view);
            xform.ApplyTransform(this._start, view);
            xform.ApplyTransform(this._current, view);
            xform.ApplyTransform(this._arc, view);
            this._circle.scaleX = this._circle.scaleY = 1.0;
            this._start.scaleY = this._current.scaleY = 4.0;
            this._arc.scaleX = this._arc.scaleY = 1.0;
            this._current.scaleX =this._start.scaleX = this._radius;
            this._start.rotation = this._dragIndicatorStartAngle;
            this._arc.rotation = this._dragIndicatorStartAngle;
            this._current.rotation = this._dragIndicatorStartAngle + this._dragDeltaAngle;

            this._circle.setActive (this._visible).setVisible(this._visible);
            this._arc.setActive    (this._visible && this._isDragging).setVisible(this._visible && this._isDragging);
            this._start.setActive  (this._visible && this._isDragging).setVisible(this._visible && this._isDragging);
            this._current.setActive(this._visible && this._isDragging).setVisible(this._visible && this._isDragging);

            return true;
        }
        
        this._arc.setActive(false).setVisible(false);
        this._circle.setActive(false).setVisible(false);
        this._start.setActive  (false).setVisible(false);
        this._current.setActive(false).setVisible(false);
        return false;
    }

    UpdateColors() {
        super.UpdateColors();
        const xform = this.transform;

        this._snapTextField._disabled = !(this._snapEnabled);// && xform != null);
        this._snapTextField.UpdateColors();

        this._UpdateTransformPosition();
    }

    Layout(x, y, width, height) {
        super.Layout(x, y, width, height);

        if (x === undefined) { x = this._x; }
        if (y === undefined) { y = this._y; }
        if (width === undefined) { width = this._width; }
        if (height === undefined) { height = this._height; }

        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        x += UIGlobals.Sizes.ToolBarShelfIndent;
        y += UIGlobals.Sizes.ToolBarShelfTextTopOffset
        const itemGap = UIGlobals.Sizes.ToolBarShelfItemsGap;

        this._snapLabel.setPosition(x, y);
        x += this._snapLabel.width + itemGap;

        this._snapCheckbox.Layout(x, y + 2);
        x += this._snapCheckbox._width + itemGap;

        this._snapTextField.Layout(x, y, 75);
        x += this._snapTextField._width + itemGap;

        this._UpdateTransformPosition();
    }

    DragStart(gameObject, pointer) {
        if (gameObject == this._circle) {
            UIGlobals.Active = gameObject;
            this._mouseDownPos.x = pointer.x;
            this._mouseDownPos.y = pointer.y;

            this._dragStartAngle = this.transform.rotation;
            this._dragDeltaAngle = 0;

            this._normalizedMouseDownPos.x = pointer.x - gameObject.x;
            this._normalizedMouseDownPos.y = pointer.y - gameObject.y;
            const lenSq = this._normalizedMouseDownPos.x * this._normalizedMouseDownPos.x + this._normalizedMouseDownPos.y * this._normalizedMouseDownPos.y;
            if (lenSq > 0.00001) {
                const len = Math.sqrt(lenSq);
                this._normalizedMouseDownPos.x /= len;
                this._normalizedMouseDownPos.y /= len;
            }
            else {
                this._normalizedMouseDownPos.x = 1;
                this._normalizedMouseDownPos.y = 0;
            }


            const right = { x: 1, y: 0 };
            let dot = right.x * this._normalizedMouseDownPos.x + right.y * this._normalizedMouseDownPos.y;
            let angle = Math.acos(dot);
            if (angle > 0) {
                const perpX = right.y;
                const perpY = right.x * -1.0;
                dot = this._normalizedMouseDownPos.x * perpX +  this._normalizedMouseDownPos.y * perpY;
                if (dot >= 0.0) {
                    angle = Math.PI * 2 - angle;
                }
            }
            this._dragIndicatorStartAngle = angle;
        }
    }

    Drag(gameObject, pointer, dragX, dragY) {
        if (gameObject == this._circle) {
            this._isDragging = true;
            const normalizedMousePos = {x: 0, y: 0};
            normalizedMousePos.x = pointer.x - gameObject.x;
            normalizedMousePos.y = pointer.y - gameObject.y;
            const lenSq = normalizedMousePos.x * normalizedMousePos.x + normalizedMousePos.y * normalizedMousePos.y;
            if (lenSq > 0.00001) {
                const len = Math.sqrt(lenSq);
                normalizedMousePos.x /= len;
                normalizedMousePos.y /= len;
            }
            else {
                normalizedMousePos.x = 0;
                normalizedMousePos.y = 0;
            }

            let dot = normalizedMousePos.x * this._normalizedMouseDownPos.x + 
                        normalizedMousePos.y * this._normalizedMouseDownPos.y;
            let angle = Math.acos(dot);
            if (angle > 0) {
                const perpX = this._normalizedMouseDownPos.y;
                const perpY = this._normalizedMouseDownPos.x * -1.0;
                dot = normalizedMousePos.x * perpX + 
                      normalizedMousePos.y * perpY;
                if (dot >= 0.0) {
                    angle = Math.PI * 2 - angle;
                }
            }

            this._dragDeltaAngle = angle;
            if (this._snapEnabled) {
                this._dragDeltaAngle = Math.floor(this._dragDeltaAngle / Phaser.Math.DegToRad(this._snapStepSize)) * Phaser.Math.DegToRad(this._snapStepSize);
            }
            //console.log("Angle: " + Phaser.Math.RadToDeg(angle));

            this._arc.clear();
            this._arc.lineStyle(Math.floor(4), 0xffff00, 1);
            this._arc.beginPath();
            this._arc.arc(0, 0, Math.round(this._radius / 4), 0, this._dragDeltaAngle, false);
            this._arc.strokePath();

            const xfrm = this.transform;
            xfrm.rotation = this._dragStartAngle + this._dragDeltaAngle;
            

            if (this._UpdateTransformPosition()) {
                this._sceneView._hierarchyView._UpdateTransforms();

                const inspector = this._sceneView._inspectorView;
                inspector.FocusOn(this._sceneView._hierarchyView._tree.selected);
            }
        }
    }

    DragEnd(gameObject, pointer) {
        if (gameObject == this._circle) {
            this._isDragging = false;
            this._dragStartAngle = 0;
            this._dragDeltaAngle = 0;
            UIGlobals.Active = null;
            this._UpdateTransformPosition();
        }
    }
}
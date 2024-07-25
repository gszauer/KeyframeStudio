import UIGlobals from './UIGlobals.js'
import UIToolBarShelf  from './UIToolBarShelf.js';
import UITextBox from './UITextBox.js'
import UIToggle from './UIToggle.js'
import XForm from './Transform.js'

export default class ScaleShelf extends UIToolBarShelf {
    _sceneView = null;

    _snapLabel = null;
    _snapCheckbox = null;
    _snapTextField = null;

    _useLocalSpace = true;
    _selectOnClick = false;
    _snapStepSize = 10;
    _snapEnabled = false;

    _dragging = false;
    _dragStart = { x: 0, y: 0 };

    _xAxis = null;
    _yAxis = null;
    _omniAxis = null;
    

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


        const arrowSize = 4;
        const arrowHeight = 16;
        const arrowTopWidth = 3;
        const arrowTopHeight = 6;
        const omniSize = 7;
        const omniSpace = 3;

        this._yAxis = scene.add.polygon(0, 0, [
            -arrowSize, /*0*//*arrowSize*/(omniSize + omniSpace),
            -arrowSize, arrowSize * arrowHeight,
            -arrowSize * arrowTopWidth, arrowSize * arrowHeight,
            -arrowSize * arrowTopWidth, arrowSize * (arrowHeight + arrowTopHeight),
            arrowSize * arrowTopWidth, arrowSize * (arrowHeight + arrowTopHeight),
            arrowSize * arrowTopWidth, arrowSize * arrowHeight,
            arrowSize, arrowSize * arrowHeight,
            arrowSize, /*0*//*-arrowSize*/(omniSize + omniSpace),
        ], 0x00ff00);
        this._yAxis.setDepth(UIGlobals.OverlayLayer - 5);
        this._yAxis.setOrigin(0, 0);
        this._yAxis.setMask(sceneView.mask);

        this._xAxis = scene.add.polygon(0, 0, [
            /*0*//*arrowSize*/omniSize + omniSpace, -arrowSize, 
            arrowSize * arrowHeight, -arrowSize, 
            arrowSize * arrowHeight, -arrowSize * arrowTopWidth, 
            arrowSize * (arrowHeight + arrowTopHeight), -arrowSize * arrowTopWidth, 
            arrowSize * (arrowHeight + arrowTopHeight), arrowSize * arrowTopWidth, 
            arrowSize * arrowHeight, arrowSize * arrowTopWidth, 
            arrowSize * arrowHeight, arrowSize, 
            /*0*//*-arrowSize*/omniSize + omniSpace, arrowSize, 
        ], 0xff0000);
        this._xAxis.setDepth(UIGlobals.OverlayLayer - 5);
        this._xAxis.setOrigin(0, 0);
        this._xAxis.setMask(sceneView.mask);

        this._xAxis.setInteractive(new Phaser.Geom.Rectangle(
            omniSize + omniSpace, 
            -arrowSize * arrowTopWidth, 
            arrowSize * (arrowHeight + arrowTopHeight) - (omniSize + omniSpace), 
            arrowSize * arrowTopWidth * 2
        ), Phaser.Geom.Rectangle.Contains);
        scene.input.setDraggable(this._xAxis);
        this._yAxis.setInteractive(new Phaser.Geom.Rectangle(
            -arrowSize * arrowTopWidth,
            (omniSize + omniSpace),
            arrowSize * arrowTopWidth * 2,
            arrowSize * (arrowHeight + arrowTopHeight)  - (omniSize + omniSpace)
        ), Phaser.Geom.Rectangle.Contains);
        scene.input.setDraggable(this._yAxis);

        this._omniAxis = scene.add.rectangle(0, 0, omniSize * 2, omniSize * 2, 0xffffff);
        this._omniAxis.setDepth(UIGlobals.OverlayLayer - 5);
        this._omniAxis.setOrigin(0.5, 0.5);
        this._omniAxis.setMask(sceneView.mask);
        this._omniAxis.setInteractive();
        scene.input.setDraggable(this._omniAxis);

        const xAxis = this._xAxis;
        const yAxis = this._yAxis;
        const omni = this._omniAxis;
        
        //console.log('x axis position', this._xAxis.x, this._xAxis.y);
        //console.log('x axis hitArea', this._xAxis.input.hitArea);
        scene.input.on('dragstart', (pointer, gameObject) => {
            if (gameObject === xAxis || gameObject === yAxis || gameObject == omni) {
                self.DragStart(gameObject, pointer);
                gameObject.fillColor = 0xffff00;
            }
        });
        scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
            if (gameObject === xAxis || gameObject === yAxis || gameObject == omni) {
                self.Drag(gameObject, pointer, dragX, dragY);
            }
        });
        scene.input.on('dragend', (pointer, gameObject) => {
            if (gameObject == xAxis) {
                self.DragEnd(gameObject, pointer);
                gameObject.fillColor = 0xff0000;
            }
            else if (gameObject == yAxis) {
                self.DragEnd(gameObject, pointer);
                gameObject.fillColor = 0x00ff00;
            }
            else if (gameObject == omni) {
                self.DragEnd(gameObject, pointer);
                gameObject.fillColor = 0xffffff;
            }
        });
    }

    SetVisibility(value) {
        super.SetVisibility(value);

        const moveIsVisible = this.transform != null;
        this._xAxis.setActive(moveIsVisible && value).setVisible(moveIsVisible && value);
        this._yAxis.setActive(moveIsVisible && value).setVisible(moveIsVisible && value);
        this._omniAxis.setActive(moveIsVisible && value).setVisible(moveIsVisible && value);

        this._snapLabel.setActive(value).setVisible(value);
        this._snapCheckbox.SetVisibility(value);
        this._snapTextField.SetVisibility(value);
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
            xform.ApplyTransform(this._xAxis, view);
            xform.ApplyTransform(this._yAxis, view);
            xform.ApplyTransform(this._omniAxis, view);
            this._xAxis.scaleX = this._yAxis.scaleX = this._omniAxis.scaleX = 1.0;
            this._xAxis.scaleY = this._yAxis.scaleY = this._omniAxis.scaleY = 1.0;
            return true;
        }
        return false;
    }

    UpdateColors() {
        super.UpdateColors();
        const xform = this.transform;

        this._snapTextField._disabled = !(this._snapEnabled);// && xform != null);
        this._snapTextField.UpdateColors();

        const moveIsVisible = xform != null && this._visible;

        this._xAxis.setActive(moveIsVisible).setVisible(moveIsVisible);
        this._yAxis.setActive(moveIsVisible).setVisible(moveIsVisible);
        this._omniAxis.setActive(moveIsVisible).setVisible(moveIsVisible);
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

    _project(a, /*onto*/ b) {
        const magBSq = b.x * b.x + b.y * b.y;
        const scale = (a.x * b.x + a.y * b.y) / magBSq;
        return {
            x: b.x * scale,
            y: b.y * scale
        }
    }

    DragStart(gameObject, pointer) {
        if (gameObject === this._xAxis || gameObject === this._yAxis || gameObject === this._omniAxis) {
            UIGlobals.Active = gameObject;
            this._dragging = true;
            let xform = this.transform;
            if (xform != null) {
                xform = xform.worldTransform;
                XForm.Mul(this.GetViewTransform(), xform, xform);
                this._dragStart.x = xform.x;
                this._dragStart.y = xform.y;
            }
        }
    }

    Drag(gameObject, pointer, dragX, dragY) {
        if (gameObject === this._xAxis || gameObject === this._yAxis || gameObject === this._omniAxis) {
            const xform = this.transform;
            if (xform != null) {
                const delta = {
                    x: pointer.x - this._dragStart.x,
                    y: pointer.y - this._dragStart.y
                };

                const scaleScale = 250;
                const lenSq = delta.x * delta.x + delta.y * delta.y;
                if (gameObject === this._xAxis || gameObject === this._omniAxis) {
                    xform.scaleX = lenSq / scaleScale;
                }
                if (gameObject === this._yAxis || gameObject === this._omniAxis) {
                    xform.scaleY = lenSq / scaleScale;
                }

                if (this._snapEnabled) {
                    xform.scaleX = Math.floor(xform.scaleX / this._snapStepSize) * this._snapStepSize;
                    xform.scaleY =  Math.floor(xform.scaleY / this._snapStepSize) * this._snapStepSize;
                }

                
                if (this._UpdateTransformPosition()) {
                    this._sceneView._hierarchyView._UpdateTransforms();

                    const inspector = this._sceneView._inspectorView;
                    inspector.FocusOn(this._sceneView._hierarchyView._tree.selected);
                }
            }
        }
    }

    DragEnd(gameObject, pointer) {
        if (gameObject === this._xAxis || gameObject === this._yAxis || gameObject === this._omniAxis) {
            this._dragging = false;
            UIGlobals.Active = null;
            if (this._UpdateTransformPosition()) {
                this._sceneView._hierarchyView._UpdateTransforms();
            }
            
            const hierarchy = this._sceneView._hierarchyView;
            if (hierarchy === null || hierarchy === undefined) {
                return;
            }
            const selected = hierarchy._tree.selected;
            if (selected === null || selected === undefined) {
                return null;
            }

            const inspector = this._sceneView._inspectorView;
            inspector.FocusOn(selected);
        }
    }

    get transform() {
        const hierarchy = this._sceneView._hierarchyView;
        if (hierarchy === null || hierarchy === undefined) {
            return null;
        }

        /*const tree = hierarchy._tree;
        if (tree === null || tree === undefined) {
            return null;
        }*/

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
}
import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UIToolBarShelf  from './UIToolBarShelf.js';
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'
import UIToggle from './UIToggle.js'
import XForm from './Transform.js'

export default class SceneView extends UIView {
    _cameraTransform = null;
    _input = null;
    
    onViewChanged = null;
    mask = null;

    _maskRect = null;
    _lines = [];
    static _numLines = 40;    // was 20
    static _lineSizing = 100; // was 200

    activeShelf = null;

    _hierarchyView = null;
    _inspectorView = null;
    _recenter = true;

    constructor(scene, parent) {
        super(scene, parent);
        const self = this;

        this._maskRect = scene.add.rectangle(0, 0, 100, 100, 0x000000).setVisible(false).setOrigin(0, 0);
        const mask = this.mask = this._maskRect.createGeometryMask();

        this._input = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._input.setDepth(-500);
        this._input.setOrigin(0, 0);
        this._input.setTint(UIGlobals.Colors.BackgroundLayer0);

        this._cameraTransform = {
            x: 0, y: 0,
            rotation: 0,
            scaleX: 1, scaleY: 1
        };

        const numLines = SceneView._numLines;
        for (let i = 0; i < numLines; ++i) {
            const line1 = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            line1.setDepth(UIGlobals.WidgetLayer);
            line1.setOrigin(0.5, 0.5);
            line1.setMask(mask);
            this._lines.push(line1);

            const line2 = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            line2.setDepth(UIGlobals.WidgetLayer);
            line2.setOrigin(0.5, 0.5);
            line2.setMask(mask);
            this._lines.push(line2);
        }

        this._input.setInteractive();
        scene.input.setDraggable(this._input);

        scene.input.on('dragstart', (pointer, gameObject) => {
            if (gameObject != self._input) { return; }
            if (self.activeShelf != null) {
                self.activeShelf.DragStart(pointer);
            }
        });
        scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
            if (gameObject != self._input) { return; }
            if (self.activeShelf != null) {
                self.activeShelf.Drag(pointer, dragX, dragY);
            }
        });
        scene.input.on('dragend', (pointer, gameObject) => {
            if (gameObject != self._input) { return; }
            if (self.activeShelf != null) {
                self.activeShelf.DragEnd(pointer);
            }
        });

        this.UpdateColors();
    }

    UpdateColors() {
        const numLines = SceneView._numLines;
        const color = UIGlobals.Colors.Dark.Gray75;
        for (let i = 0; i < numLines; ++i) {
            this._lines[i * 2 + 0].setTint(color);
            this._lines[i * 2 + 1].setTint(color);
        }
        const centerIndex = Math.floor(numLines / 2);

        this._lines[centerIndex * 2 + 0].setTint(UIGlobals.Colors.Dark.Gray100);
        this._lines[centerIndex * 2 + 1].setTint(UIGlobals.Colors.Dark.Gray100);

        if (this.activeShelf != null) {
            this.activeShelf.UpdateColors();
        }
    }

    Layout(x, y, width, height) {
        if (x === undefined) { x = this._x; }
        if (y === undefined) { y = this._y; }
        if (width === undefined) { width = this._width; }
        if (height === undefined) { height = this._height; }
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        if (this._recenter) {
            this._cameraTransform.x = width / 2;
            this._cameraTransform.y = height / 2;
            this._recenter = false;
        }

        this._input.setPosition(x, y);
        this._input.setScale(width, height);

        this._maskRect.setPosition(x, y);
        this._maskRect.setSize(width, height)

        const spacing = SceneView._lineSizing;
        const gridLineSize = 2;
        const highlightedGridLineSize = 4;

        const globalXOffset = UIGlobals.Sizes.ToolboxWidth;
        const globalYOffset = UIGlobals.Sizes.EditorBarHeight;

        const numLines = SceneView._numLines;
        for (let i = 0; i < numLines; ++i) {
            let _x = i * spacing - ((numLines / 2) * spacing);
            
            _x *= this._cameraTransform.scaleX;
            _x += this._cameraTransform.x;
            _x += globalXOffset;

            let _y = i * spacing - ((numLines / 2) * spacing);
            
            _y *= this._cameraTransform.scaleY;
            _y += this._cameraTransform.y;
            _y += globalYOffset;

            this._lines[i * 2 + 0].setPosition(_x, 0);
            this._lines[i * 2 + 0].setScale(gridLineSize, numLines * spacing);

            this._lines[i * 2 + 1].setPosition(0, _y);
            this._lines[i * 2 + 1].setScale(numLines * spacing, gridLineSize);
        }

        const centerIndex = Math.floor(numLines / 2);
        this._lines[centerIndex * 2 + 0].setScale(highlightedGridLineSize, numLines * spacing);
        this._lines[centerIndex * 2 + 1].setScale(numLines * spacing, highlightedGridLineSize);

        this._hierarchyView._UpdateTransforms();
    }

    SetVisibility(value) {
        const numLines = SceneView._numLines;
        for (let i = 0; i < numLines; ++i) {
            this._lines[i * 2 + 0].setActive(value).setVisible(value);
            this._lines[i * 2 + 1].setActive(value).setVisible(value);
        }
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }

    CreateToolShelves(toolbar) {
        const move = new MoveShelf(this._scene, this);
        toolbar.AddShelf(UIGlobals.IconMove, move);

        const scale = new ScaleShelf(this._scene, this);
        toolbar.AddShelf(UIGlobals.IconScale, scale);

        const pan = new PanShelf(this._scene, toolbar, this);
        toolbar.AddShelf(UIGlobals.IconHand, pan);

        const zoom = new ZoomShelf(this._scene, toolbar, this);
        toolbar.AddShelf(UIGlobals.IconZoomIn, zoom);
    }

    UpdateActiveShelf() {
        if (this.activeShelf) {
            this.activeShelf._UpdateTransformPosition();
        }
    }
}

export class PanShelf extends UIToolBarShelf {
    _sceneView = null;
    _viewportXLabel = null;
    _viewportXInput = null;

    _viewportYLabel = null;
    _viewportYInput = null;

    _zoomLabel = null;
    _zoomInput = null;

    _gridLabel = null;
    _gridInput = null;

    _cameraStart = null;
    _dragStart = null; // Pointer

    constructor(scene, toolbar, sceneView) {
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

        this._dragStart = {x: 0,  y: 0, scaleX: 1, scaleY: 1};
        this._cameraStart = { x: 0, y: 0, scaleX: 1, scaleY: 1 };

        const viewportXLabel = this._viewportXLabel = 
            scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        viewportXLabel.setDepth(UIGlobals.WidgetLayer);
        viewportXLabel.text = "Viewport  x:";

        const viewPortXInput = this._viewportXInput = new UITextBox(scene, "0");
        viewPortXInput.onTextEdit = (value) => {
            self._sceneView._cameraTransform.x = Number(NumerisizeString(value));
            self._sceneView.Layout();
        };

        const viewportYLabel = this._viewportYLabel = 
            scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        viewportYLabel.setDepth(UIGlobals.WidgetLayer);
        viewportYLabel.text = "  y:";

        const viewPortYInput = this._viewportYInput = new UITextBox(scene, "0");
        viewPortYInput.onTextEdit = (value) => {
            self._sceneView._cameraTransform.y = Number(NumerisizeString(value));
            self._sceneView.Layout();
        };

        const zoomLabel = this._zoomLabel = 
            scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        zoomLabel.setDepth(UIGlobals.WidgetLayer);
        zoomLabel.text = "     Zoom";

        const zoomInput = this._zoomInput = new UITextBox(scene, "0");
        zoomInput.onTextEdit = (value) => {
            const scale = Number(NumerisizeString(value))
            self._sceneView._cameraTransform.scaleX = scale;
            self._sceneView._cameraTransform.scaleY = scale;
            self._sceneView.Layout();
        };

        const gridLabel = this._gridLabel =  scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        gridLabel.setDepth(UIGlobals.WidgetLayer);
        gridLabel.text = "     Grid";

        let popup = new UIPopup(scene);
        popup.Add("Show", () => {
            sceneView.Show();
        });
        popup.Add("Hide", () => {
            sceneView.Hide();
        });

        const gridInput = this._gridInput = new UIDropdown(scene, popup);
    }

    UpdateColors() {
    }

    _UpdateTransformPosition() {}

    SetVisibility(value) {
        super.SetVisibility(value);
        this._viewportXLabel.setActive(value).setVisible(value);
        this._viewportXInput.SetVisibility(value);
        this._viewportYLabel.setActive(value).setVisible(value);
        this._viewportYInput.SetVisibility(value);
        this._zoomLabel.setActive(value).setVisible(value);
        this._zoomInput.SetVisibility(value);
        this._gridLabel.setActive(value).setVisible(value);
        this._gridInput.SetVisibility(value);

        const truncateFloat = (str, digits) => {
            let re = new RegExp("(\\d+\\.\\d{" + digits + "})(\\d)"),
                m = str.toString().match(re);
            return m ? parseFloat(m[1]) : str.valueOf();
        };

        if (value) {
            this._viewportXInput.text = truncateFloat("" + this._sceneView._cameraTransform.x, 3);
            this._viewportYInput.text = truncateFloat("" + this._sceneView._cameraTransform.y, 3);
            this._zoomInput.text = truncateFloat("" + this._sceneView._cameraTransform.scaleY, 3);
        }
    }

    Layout(x, y, width, height) {
        super.Layout(x, y, width, height);
        x = this._x; y = this._y;
        height = this._height;
        width = this._width;

        const itemGap = UIGlobals.Sizes.ToolBarShelfItemsGap;
        const textBoxWidth = UIGlobals.Sizes.ToolBarShelfTextBoxWidth;

        x += UIGlobals.Sizes.ToolBarShelfIndent;
        y += UIGlobals.Sizes.ToolBarShelfTextTopOffset

        this._viewportXLabel.setPosition(x, y);
        x += this._viewportXLabel.width + itemGap;

        this._viewportXInput.Layout(x, y, textBoxWidth, 20);
        x += textBoxWidth + itemGap;

        this._viewportYLabel.setPosition(x, y);
        x += this._viewportYLabel.width + itemGap;

        this._viewportYInput.Layout(x, y, textBoxWidth, 20);
        x += textBoxWidth + itemGap;

        this._zoomLabel.setPosition(x, y);
        x += this._zoomLabel.width + itemGap;

        this._zoomInput.Layout(x, y, textBoxWidth);
        x += textBoxWidth + itemGap;

        this._gridLabel.setPosition(x, y);
        x += this._gridLabel.width + itemGap;

        this._gridInput.Layout(x, y, textBoxWidth);
        x += textBoxWidth + itemGap;
    }

    DragStart(pointer) {
        this._dragStart.x = pointer.worldX;
        this._dragStart.y = pointer.worldY;

        this._cameraStart.x = this._sceneView._cameraTransform.x;
        this._cameraStart.y = this._sceneView._cameraTransform.y;
        this._cameraStart.scaleX = this._sceneView._cameraTransform.scaleX;
        this._cameraStart.scaleY = this._sceneView._cameraTransform.scaleY;
    }

    Drag(pointer, dragX, dragY) {
        const deltaX = pointer.worldX - this._dragStart.x;
        const deltaY = pointer.worldY - this._dragStart.y;

        this._sceneView._cameraTransform.x = this._cameraStart.x + deltaX;
        this._sceneView._cameraTransform.y = this._cameraStart.y + deltaY;

        const truncateFloat = (str, digits) => {
            let re = new RegExp("(\\d+\\.\\d{" + digits + "})(\\d)"),
                m = str.toString().match(re);
            return m ? parseFloat(m[1]) : str.valueOf();
        };

        this._zoomInput.text = + truncateFloat("" + this._cameraStart.scaleY, 3);
        this._viewportXInput.text = truncateFloat("" + this._sceneView._cameraTransform.x, 3);
        this._viewportYInput.text = truncateFloat("" + this._sceneView._cameraTransform.y, 3);

        this._sceneView.Layout();
    }

    DragEnd(pointer) {

    }
}

export class ZoomShelf extends PanShelf {
    DragStart(pointer) {
        this._dragStart.x = pointer.worldX;
        this._dragStart.y = pointer.worldY;

        this._cameraStart.x = this._sceneView._cameraTransform.x;// * this._sceneView._cameraTransform.scaleX;
        this._cameraStart.y = this._sceneView._cameraTransform.y;// * this._sceneView._cameraTransform.scaleY;
        this._cameraStart.scaleX = this._sceneView._cameraTransform.scaleX;
        this._cameraStart.scaleY = this._sceneView._cameraTransform.scaleY;
    }

    Drag(pointer, dragX, dragY) {
        const deltaX = (pointer.worldX - this._dragStart.x) * 0.01;
        const deltaY = (pointer.worldY - this._dragStart.y) * 0.01;
        
        const scaleX = Math.abs(this._cameraStart.scaleX + deltaX);
        const scaleY = Math.abs(this._cameraStart.scaleY + deltaY);

        this._sceneView._cameraTransform.scaleX = scaleY;
        this._sceneView._cameraTransform.scaleY = scaleY;

        //this._sceneView._cameraTransform.x = this._cameraStart.x + (this._cameraStart.x * scaleY - this._cameraStart.x);
        //this._sceneView._cameraTransform.y = this._cameraStart.y + (this._cameraStart.y * scaleY - this._cameraStart.y);
 
        const truncateFloat = (str, digits) => {
            let re = new RegExp("(\\d+\\.\\d{" + digits + "})(\\d)"),
                m = str.toString().match(re);
            return m ? parseFloat(m[1]) : str.valueOf();
        };

        this._zoomInput.text = + truncateFloat("" + scaleY, 3);
        this._viewportXInput.text = truncateFloat("" + this._sceneView._cameraTransform.x, 3);
        this._viewportYInput.text = truncateFloat("" + this._sceneView._cameraTransform.y, 3);

        this._sceneView.Layout();
    }

    DragEnd(pointer) {

    }
}

export class MoveShelf extends UIToolBarShelf {
    _sceneView = null;

    _spaceLabel = null;
    _spaceDropdown = null;

    _snapLabel = null;
    _snapCheckbox = null;
    _snapTextField = null;

    _useLocalSpace = true;
    _selectOnClick = false;
    _snapStepSize = 10;

    _dragging = false;
    _dragOffset = { x: 0, y: 0 };

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

        this._spaceLabel = scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        this._spaceLabel.setDepth(UIGlobals.WidgetLayer);
        this._spaceLabel.text = "Space: ";

        let popup = new UIPopup(scene);
        popup.Add("Local", () => {
            self._useLocalSpace = true;
            // TODO
        });
        popup.Add("World", () => {
            self._useLocalSpace = false;
            // TODO
        });
        this._spaceDropdown = new UIDropdown(scene, popup);

        this._snapLabel = scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        this._snapLabel.setDepth(UIGlobals.WidgetLayer);
        this._snapLabel.text = "     Snap:";

        this._snapCheckbox = new UIToggle(scene, "", (valueBol, uiToggleObject) => {
            // TODO
        });

        this._snapTextField = new UITextBox(scene, "" + this._snapStepSize);
        this._snapTextField.onTextEdit = (value) => {
            self._snapStepSize = Number(NumerisizeString(value));
            // TODO
        };

        const arrowSize = 4;
        const arrowHeight = 16;
        const arrowTopWidth = 3;
        const arrowTopHeight = 7;
        const omniSize = 7;
        const omniSpace = 3;

        this._yAxis = scene.add.polygon(0, 0, [
            -arrowSize, /*0*//*arrowSize*/-(omniSize + omniSpace),
            -arrowSize, -arrowSize * arrowHeight,
            -arrowSize * arrowTopWidth, -arrowSize * arrowHeight,
            0, -arrowSize * (arrowHeight + arrowTopHeight),
            arrowSize * arrowTopWidth, -arrowSize * arrowHeight,
            arrowSize, -arrowSize * arrowHeight,
            arrowSize, /*0*//*-arrowSize*/-(omniSize + omniSpace),
        ], 0x00ff00);
        this._yAxis.setDepth(UIGlobals.OverlayLayer - 5);
        this._yAxis.setOrigin(0, 0);

        this._xAxis = scene.add.polygon(0, 0, [
            /*0*//*arrowSize*/omniSize + omniSpace, -arrowSize, 
            arrowSize * arrowHeight, -arrowSize, 
            arrowSize * arrowHeight, -arrowSize * arrowTopWidth, 
            arrowSize * (arrowHeight + arrowTopHeight), 0, 
            arrowSize * arrowHeight, arrowSize * arrowTopWidth, 
            arrowSize * arrowHeight, arrowSize, 
            /*0*//*-arrowSize*/omniSize + omniSpace, arrowSize, 
        ], 0xff0000);
        this._xAxis.setDepth(UIGlobals.OverlayLayer - 5);
        this._xAxis.setOrigin(0, 0);

        this._xAxis.setInteractive(new Phaser.Geom.Rectangle(
            omniSize + omniSpace, 
            -arrowSize * arrowTopWidth, 
            arrowSize * (arrowHeight + arrowTopHeight) - (omniSize + omniSpace), 
            arrowSize * arrowTopWidth * 2
        ), Phaser.Geom.Rectangle.Contains);
        scene.input.setDraggable(this._xAxis);
        this._yAxis.setInteractive(new Phaser.Geom.Rectangle(
            -arrowSize * arrowTopWidth,
            -arrowSize * (arrowHeight + arrowTopHeight),
            arrowSize * arrowTopWidth * 2,
            arrowSize * (arrowHeight + arrowTopHeight) - (omniSize + omniSpace)
        ), Phaser.Geom.Rectangle.Contains);
        scene.input.setDraggable(this._yAxis);

        this._omniAxis = scene.add.rectangle(0, 0, omniSize * 2, omniSize * 2, 0xffffff);
        this._omniAxis.setDepth(UIGlobals.OverlayLayer - 5);
        this._omniAxis.setOrigin(0.5, 0.5);
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

        this._spaceLabel.setActive(value).setVisible(value);
        this._spaceDropdown.SetVisibility(value);
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

        this._spaceLabel.setPosition(x, y);
        x += this._spaceLabel.width + itemGap;

        this._spaceDropdown.Layout(x, y, 90);
        x += this._spaceDropdown._width + itemGap;

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
            this._dragging = true;
        }

        if (gameObject === this._xAxis) {
            this._dragOffset.x = this._xAxis.x - pointer.x;
            this._dragOffset.y = this._xAxis.y - pointer.y;
        }
        else if (gameObject === this._yAxis) {
            this._dragOffset.x = this._yAxis.x - pointer.x;
            this._dragOffset.y = this._yAxis.y - pointer.y;
        }
        if (gameObject === this._omniAxis) {
            this._dragOffset.x = this._omniAxis.x - pointer.x;
            this._dragOffset.y = this._omniAxis.y - pointer.y;
        }
    }

    Drag(gameObject, pointer, dragX, dragY) {
        if (gameObject === this._xAxis || gameObject === this._yAxis || gameObject === this._omniAxis) {
            const xform = this.transform;
            if (xform != null) {
                if (gameObject === this._xAxis) {
                    const deltaMotion = {
                        x: (pointer.x + this._dragOffset.x) - this._xAxis.x,
                        y: (pointer.y + this._dragOffset.y) - this._xAxis.y
                    };

                    const world = xform.worldTransform;
                    
                    if (xform.parent != null) {
                        const parentWorld = xform.parent.worldTransform;
                        const invParentWorld = XForm.Inverse(parentWorld);
                        XForm.Mul(invParentWorld, world, world);
                    }

                    XForm.Mul(this.GetViewTransform(), world, world);
                    let constrained = this._project(deltaMotion, XForm.Right(world));

                    xform.x += constrained.x;
                    xform.y += constrained.y;
                }
                if (gameObject === this._yAxis) {
                    const deltaMotion = {
                        x: (pointer.x + this._dragOffset.x) - this._yAxis.x,
                        y: (pointer.y + this._dragOffset.y) - this._yAxis.y
                    };

                    const world = xform.worldTransform;
                    if (xform.parent != null) {
                        const parentWorld = xform.parent.worldTransform;
                        const invParentWorld = XForm.Inverse(parentWorld);
                        XForm.Mul(invParentWorld, world, world);
                    }
                    XForm.Mul(this.GetViewTransform(), world, world);
                    let constrained = this._project(deltaMotion, XForm.Up(world));

                    xform.x += constrained.x;
                    xform.y += constrained.y;
                }
                if (gameObject === this._omniAxis) {
                    xform.x += (pointer.x + this._dragOffset.x) - this._omniAxis.x;
                    xform.y += (pointer.y + this._dragOffset.y) - this._omniAxis.y;
                }
                if (this._UpdateTransformPosition()) {
                    this._sceneView._hierarchyView._UpdateTransforms();
                }
            }
        }
    }

    DragEnd(gameObject, pointer) {
        if (gameObject === this._xAxis || gameObject === this._yAxis || gameObject === this._omniAxis) {
            this._dragging = false;
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

export class ScaleShelf extends UIToolBarShelf {
    _sceneView = null;

    _snapLabel = null;
    _snapCheckbox = null;
    _snapTextField = null;

    _useLocalSpace = true;
    _selectOnClick = false;
    _snapStepSize = 10;

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
            // TODO
        });

        this._snapTextField = new UITextBox(scene, "" + this._snapStepSize);
        this._snapTextField.onTextEdit = (value) => {
            self._snapStepSize = Number(NumerisizeString(value));
            // TODO
        };

        const arrowSize = 4;
        const arrowHeight = 16;
        const arrowTopWidth = 3;
        const arrowTopHeight = 6;
        const omniSize = 7;
        const omniSpace = 3;

        this._yAxis = scene.add.polygon(0, 0, [
            -arrowSize, /*0*//*arrowSize*/-(omniSize + omniSpace),
            -arrowSize, -arrowSize * arrowHeight,
            -arrowSize * arrowTopWidth, -arrowSize * arrowHeight,
            -arrowSize * arrowTopWidth, -arrowSize * (arrowHeight + arrowTopHeight),
            arrowSize * arrowTopWidth, -arrowSize * (arrowHeight + arrowTopHeight),
            arrowSize * arrowTopWidth, -arrowSize * arrowHeight,
            arrowSize, -arrowSize * arrowHeight,
            arrowSize, /*0*//*-arrowSize*/-(omniSize + omniSpace),
        ], 0x00ff00);
        this._yAxis.setDepth(UIGlobals.OverlayLayer - 5);
        this._yAxis.setOrigin(0, 0);

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

        this._xAxis.setInteractive(new Phaser.Geom.Rectangle(
            omniSize + omniSpace, 
            -arrowSize * arrowTopWidth, 
            arrowSize * (arrowHeight + arrowTopHeight) - (omniSize + omniSpace), 
            arrowSize * arrowTopWidth * 2
        ), Phaser.Geom.Rectangle.Contains);
        scene.input.setDraggable(this._xAxis);
        this._yAxis.setInteractive(new Phaser.Geom.Rectangle(
            -arrowSize * arrowTopWidth,
            -arrowSize * (arrowHeight + arrowTopHeight),
            arrowSize * arrowTopWidth * 2,
            arrowSize * (arrowHeight + arrowTopHeight) - (omniSize + omniSpace)
        ), Phaser.Geom.Rectangle.Contains);
        scene.input.setDraggable(this._yAxis);

        this._omniAxis = scene.add.rectangle(0, 0, omniSize * 2, omniSize * 2, 0xffffff);
        this._omniAxis.setDepth(UIGlobals.OverlayLayer - 5);
        this._omniAxis.setOrigin(0.5, 0.5);
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
        this._dragging = true;
        
        if (gameObject === this._xAxis || gameObject === this._yAxis || gameObject === this._omniAxis) {
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

                

                /*if (gameObject === this._xAxis) {
                    const deltaMotion = {
                        x: (pointer.x + this._dragOffset.x) - this._xAxis.x,
                        y: (pointer.y + this._dragOffset.y) - this._xAxis.y
                    };

                    const world = xform.worldTransform;
                    
                    if (xform.parent != null) {
                        const parentWorld = xform.parent.worldTransform;
                        const invParentWorld = XForm.Inverse(parentWorld);
                        XForm.Mul(invParentWorld, world, world);
                    }

                    XForm.Mul(this.GetViewTransform(), world, world);
                    let constrained = this._project(deltaMotion, XForm.Right(world));

                    xform.x += constrained.x;
                    xform.y += constrained.y;
                }
                if (gameObject === this._yAxis) {
                    const deltaMotion = {
                        x: (pointer.x + this._dragOffset.x) - this._yAxis.x,
                        y: (pointer.y + this._dragOffset.y) - this._yAxis.y
                    };

                    const world = xform.worldTransform;
                    if (xform.parent != null) {
                        const parentWorld = xform.parent.worldTransform;
                        const invParentWorld = XForm.Inverse(parentWorld);
                        XForm.Mul(invParentWorld, world, world);
                    }
                    XForm.Mul(this.GetViewTransform(), world, world);
                    let constrained = this._project(deltaMotion, XForm.Up(world));

                    xform.x += constrained.x;
                    xform.y += constrained.y;
                }
                if (gameObject === this._omniAxis) {
                    xform.x += (pointer.x + this._dragOffset.x) - this._omniAxis.x;
                    xform.y += (pointer.y + this._dragOffset.y) - this._omniAxis.y;
                }*/
                if (this._UpdateTransformPosition()) {
                    this._sceneView._hierarchyView._UpdateTransforms();
                }
            }
        }
    }

    DragEnd(gameObject, pointer) {
        if (gameObject === this._xAxis || gameObject === this._yAxis || gameObject === this._omniAxis) {
            this._dragging = false;
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
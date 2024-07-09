import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UIToolBarShelf  from './UIToolBarShelf.js';
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'
import UIToggle from './UIToggle.js'

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

        const pan = new PanShelf(this._scene, toolbar, this);
        toolbar.AddShelf(UIGlobals.IconHand, pan);
        const zoom = new ZoomShelf(this._scene, toolbar, this);
        toolbar.AddShelf(UIGlobals.IconZoomIn, zoom);
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

    _selectLabel = null;
    _selectCheckbox = null;

    _useLocalSpace = true;
    _selectOnClick = false;
    _snapStepSize = 10;


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

        this._selectLabel = scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        this._selectLabel.setDepth(UIGlobals.WidgetLayer);
        this._selectLabel.text = "     Auto select:";

        this._selectCheckbox = new UIToggle(scene, "", (valueBol, uiToggleObject) => {
            // TODO
        });
    }

    SetVisibility(value) {
        super.SetVisibility(value);

        this._spaceLabel.setActive(value).setVisible(value);
        this._spaceDropdown.SetVisibility(value);
        this._snapLabel.setActive(value).setVisible(value);
        this._snapCheckbox.SetVisibility(value);
        this._snapTextField.SetVisibility(value);
        this._selectLabel.setActive(value).setVisible(value);
        this._selectCheckbox.SetVisibility(value);
    }

    UpdateColors() {
        super.UpdateColors();
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

        this._selectLabel.setPosition(x, y);
        x += this._selectLabel.width + itemGap;

        this._selectCheckbox.Layout(x, y + 2);
    }
}
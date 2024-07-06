import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UIToolBarShelf  from './UIToolBarShelf.js';
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'

export default class SceneView extends UIView {
    _cameraTransform = null;
    
    onViewChanged = null;
    mask = null;

    _maskRect = null;
    _lines = [];
    static _numLines = 20;

    constructor(scene, parent) {
        super(scene, parent);
        const self = this;

        this._maskRect = scene.add.rectangle(0, 0, 100, 100, 0x000000).setVisible(false).setOrigin(0, 0);
        const mask = this.mask = this._maskRect.createGeometryMask();

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
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        this._maskRect.setPosition(x, y);
        this._maskRect.setSize(width, height)

        const spacing = 200;
        const gridLineSize = 2;
        const highlightedGridLineSize = 4;

        const numLines = SceneView._numLines;
        for (let i = 0; i < numLines; ++i) {
            let _x = i * spacing - ((numLines / 2) * spacing);
            _x += x + width / 2;

            let _y = i * spacing - ((numLines / 2) * spacing);
            _y += y + height / 2;

            this._lines[i * 2 + 0].setPosition(_x, 0);
            this._lines[i * 2 + 0].setScale(gridLineSize, numLines * spacing);

            this._lines[i * 2 + 1].setPosition(0, _y);
            this._lines[i * 2 + 1].setScale(numLines * spacing, gridLineSize);
        }

        const centerIndex = Math.floor(numLines / 2);
        this._lines[centerIndex * 2 + 0].setScale(highlightedGridLineSize, numLines * spacing);
        this._lines[centerIndex * 2 + 1].setScale(numLines * spacing, highlightedGridLineSize);
    }

    SetVisibility(value) {

    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }

    CreateToolShelves(toolbar) {
        const pan = new PanShelf(this._scene, toolbar);
        toolbar.AddShelf(UIGlobals.IconHand, pan);
        toolbar.AddShelf(UIGlobals.IconZoomIn, pan);
    }
}

export class PanShelf extends UIToolBarShelf {
    _viewportXLabel = null;
    _viewportXInput = null;

    _viewportYLabel = null;
    _viewportYInput = null;

    _zoomLabel = null;
    _zoomInput = null;

    _gridLabel = null;
    _gridInput = null;

    constructor(scene, toolbar) {
        super(scene);
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

        const viewportXLabel = this._viewportXLabel = 
            scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        viewportXLabel.setDepth(UIGlobals.WidgetLayer);
        viewportXLabel.text = "Viewport  x:";

        const viewPortXInput = this._viewportXInput = new UITextBox(scene, "0");
        viewPortXInput.onTextEdit = (value) => {
            // TODO
        };

        const viewportYLabel = this._viewportYLabel = 
            scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        viewportYLabel.setDepth(UIGlobals.WidgetLayer);
        viewportYLabel.text = "  y:";

        const viewPortYInput = this._viewportYInput = new UITextBox(scene, "0");
        viewPortYInput.onTextEdit = (value) => {
            // TODO
        };

        const zoomLabel = this._zoomLabel = 
            scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        zoomLabel.setDepth(UIGlobals.WidgetLayer);
        zoomLabel.text = "     zoom";

        const zoomInput = this._zoomInput = new UITextBox(scene, "0");
        zoomInput.onTextEdit = (value) => {
            // TODO
        };

        const gridLabel = this._gridLabel = 
            scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        gridLabel.setDepth(UIGlobals.WidgetLayer);
        gridLabel.text = "     grid";

        let popup = new UIPopup(scene);
        popup.Add("Show");
        popup.Add("Hide");

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
}
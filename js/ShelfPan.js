import UIGlobals from './UIGlobals.js'
import UIToolBarShelf  from './UIToolBarShelf.js';
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'

export default class PanShelf extends UIToolBarShelf {
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
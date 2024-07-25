import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'

import ScaleShelf from './ShelfScale.js'
import RotateShelf from './ShelfRotate.js'
import PanShelf from './ShelfPan.js'
import ZoomShelf from './ShelfZoom.js'
import MoveShelf from './ShelfMove.js'

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
    _assetsView = null;
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
            this._lines[i * 2 + 0].setScale(gridLineSize, numLines * spacing * 0.5);

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

        const rotate = new RotateShelf(this._scene, this);
        toolbar.AddShelf(UIGlobals.IconRotate, rotate);

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




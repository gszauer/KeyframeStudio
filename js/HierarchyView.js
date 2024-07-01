import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITree from './UITree.js'
import UIImageButton from './UIImageButton.js'
import XForm from './Transform.js'

export default class HierarchyView extends UIView {
    static _nameCounter = 0;
    _tree = null;
    _footer = null;
    _buttons = [];
    _active = null;

    onSelectionChanged = null; // (oldActive, newActive)

    get active() {
        return this._active;
    }

    set active(valeu) {
        if (this._active != valeu) {
            if (this.onSelectionChanged != null) {
                this.onSelectionChanged(this._active, valeu);
            }
        }
        this._active = valeu;
    }

    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._tree = new UITree(scene);
        this._tree.canReorder = false;

        this._tree.onSelected = (treeNode) => {
            self.active = treeNode;
        }

        this._footer =  scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._footer.setDepth(UIGlobals.WidgetLayer);
        this._footer.setOrigin(0, 0);

        const newNodeButton = new UIImageButton(scene, "SmallIconHierarchyNew.png", () => {
            const hierarchyNode = self.AddNewNode();
            const transformNode = new XForm(hierarchyNode);
        });
        const deleteNodeButton = new UIImageButton(scene, "SmallIconTrash.png", () => {
            self.Delete();
        });
        const deselectButton = new UIImageButton(scene, "SmallIconDeselect.png", () => {
            self.Deselect();
        });
        this._buttons.push(newNodeButton);
        this._buttons.push(deleteNodeButton);
        this._buttons.push(deselectButton);
    }

    Delete() {
        if (this.active == null) {
            return;
        }
        const toRemove = this.active;
        this.Deselect();
        this._tree.Remove(toRemove);
    }

    Deselect() {
        this._tree.Deselect();
        this.active = null;
    }

    AddNewNode(nodeName) {
        if (!nodeName) {
            nodeName = "Unnamed Node " + (HierarchyView._nameCounter++);;
        }

        let parent = null;
        if (this.active) {
            parent = this.active;
        }
        
        const newNode = this._tree.Add(nodeName);
        if (parent) {
            newNode.SetParent(parent);
        }

        this.Layout(this._x, this._y, this._width, this._height);
        return newNode;
    }

    UpdateColors() {
        this._tree.UpdateColors();
        this._footer.setTint(UIGlobals.Colors.BackgroundLayer0);
        const length = this._buttons.length;
        for (let i = 0; i < length; ++i) {
            this._buttons[i].UpdateColors();
        }
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        const footerHeight = UIGlobals.Sizes.TreeFooterHeight;
        const size = footerHeight - UIGlobals.Sizes.TreeFooterMargin * 2;
        const padding = UIGlobals.Sizes.TreeFooterPadding;
        const margin = UIGlobals.Sizes.TreeFooterMargin;

        this._tree.Layout(x, y + footerHeight, width, height - footerHeight);

        this._footer.setPosition(x, y);
        this._footer.setScale(width, footerHeight);

        let xPos = x + padding + (size / 2);
        let yPos = y + margin + (size / 2);

        const length = this._buttons.length;
        for (let i = 0; i < length; ++i) {
            this._buttons[i].Layout(xPos, yPos, size, size);
            xPos += (size + padding);
        }
    }

    SetVisibility(value) {
        this._tree.SetVisibility(value);
        this._footer.setActive(value).setVisible(value);

        const length = this._buttons.length;
        for (let i = 0; i < length; ++i) {
            this._buttons[i].SetVisibility(value);
        }
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
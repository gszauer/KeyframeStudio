import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITree from './UITree.js'

export default class HierarchyView extends UIView {
    static _nameCounter = 0;
    _tree = null;
    _footer = null;
    _active = null;

    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._tree = new UITree(scene);
        this._tree.canReorder = false;

        this._tree.onSelected = (treeNode) => {
            self._active = treeNode;
        }
    }

    AddNewNode(nodeName) {
        if (!nodeName) {
            nodeName = "Unnamed Node " + (HierarchyView._nameCounter++);;
        }

        let parent = null;
        if (this._active) {
            parent = this._active;
        }
        
        const newNode = this._tree.Add(nodeName);
        if (parent) {
            newNode.SetParent(parent);
        }
        //newNode.

        this.Layout(this._x, this._y, this._width, this._height);
        return newNode;
    }

    UpdateColors() {
        this._tree.UpdateColors();
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);
        this._tree.Layout(x, y, width, height - UIGlobals.Sizes.TreeFooterHeight);
    }

    SetVisibility(value) {
        this._tree.SetVisibility(value);
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
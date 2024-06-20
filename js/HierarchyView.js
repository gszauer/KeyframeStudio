import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITree from './UITree.js'

export default class HierarchyView extends UIView {
    _tree = null;

    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._tree = new UITree(scene);
        this._tree.canReorder = false;

        let one = this._tree.Add("One");
        let two = this._tree.Add("Two");
        let three = this._tree.Add("Three");
        let four = this._tree.Add("Four");

        one.AddChild(two);
        three.SetParent(four);
        three.SetParent(two);

        for (let i = 0; i < 20; ++i) {
            this._tree.Add("Hierarchy " + i);
        }
    }

    UpdateColors() {
        this._tree.UpdateColors();
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);
        this._tree.Layout(x, y, width, height);
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
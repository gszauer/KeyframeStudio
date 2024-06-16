import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UIScrollView from './UIScrollView.js'
import UIListBox from './UIListBox.js'
import UIListBoxItem from './UIListBoxItem.js'

export default class DrawOrderView extends UIView {
    _listbox = null;

    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._listbox = new UIListBox(scene);

        this._listbox.Add("item 0");
        this._listbox.Add("item 1");
        this._listbox.Add("item 2");
        this._listbox.Add("item 3");
        this._listbox.Add("item 4");
        this._listbox.Add("item 5");
        this._listbox.Clear();
        this._listbox.Add("Item 5");
        this._listbox.Add("Item 4");
        this._listbox.Add("Item 3");
        this._listbox.Clear();
        for (let i = 0; i < 20; ++i) {
            this._listbox.Add("Item " + i);
        }
        console.log(this._listbox._buttons.length + " / 20");
    }

    UpdateColors() {
        this._listbox.UpdateColors();
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);
        this._listbox.Layout(x, y, width, height);
    }

    SetVisibility(value) {
        this._listbox.SetVisibility(value);
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
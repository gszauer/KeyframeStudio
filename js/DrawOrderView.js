import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UIScrollView from './UIScrollView.js'
import UIListBox from './UIListBox.js'
import UIListBoxItem from './UIListBoxItem.js'

export default class DrawOrderView extends UIView {
    _listbox = null;
    _hierarchyView = null; // Injected when hiararchy view is created

    get count() {
        return this._listbox.count;
    }

    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._listbox = new UIListBox(scene);
        this._listbox.onReodered = (startIndex, stopIndex, delta) => {
            if (self._hierarchyView == null) {
                return;
            }
            
            const hierarchy = self._hierarchyView;
            const listBoxButtons = self._listbox._buttons;
            const count = listBoxButtons.length;

            for (let i = 0; i < count; ++i) {
                const uiListBoxItem = listBoxButtons[i];
                hierarchy.UpdateSortingIndex(uiListBoxItem, UIGlobals.WidgetLayer - i);
            }
        }
    }

    Add(name, callback = null) {
        return this._listbox.Add(name, callback);
    }

    UpdateColors() {
        this._listbox.UpdateColors();
    }

    Layout(x, y, width, height) {
        if(x === undefined) { x = this._x; }
        if (y === undefined) { y = this._y; }
        if (width === undefined) { width = this._width; }
        if (height === undefined) { height = this._height; }
        
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
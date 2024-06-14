import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UIScrollView from './UIScrollView.js'
import UIListBoxItem from './UIListBoxItem.js'

export default class DrawOrderView extends UIView {
    _scrollView = null;
    _buttons = null;

    _elements = null;


    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._scrollView = new UIScrollView(scene, this);
        this._scrollView.showHorizontal = false;
        
        this._elements = [];
        for (let i = 0; i < 10; ++i) {
            this._elements.push(new UIListBoxItem(scene, "Test Item " + (i + 1)));
            this._elements[i].AddToContainer(this._scrollView.container);
        }
    }

    UpdateColors() {
        this._scrollView.UpdateColors();

        const tnitColors = [
            UIGlobals.Colors.BackgroundLayer1,
            UIGlobals.Colors.BackgroundLayer1AndAHalf
        ];
        for (let i = 0; i < 10; ++i) {
            this._elements[i].SetTint(tnitColors[i % 2]);
        }
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        this._scrollView.Layout(x, y, width, height);
        const buttonHeight = UIGlobals.Sizes.ListBoxItemHeight;

        x = 0;
        y = 0;
        width = this._scrollView._width;
        for (let i = 0; i < 10; ++i) {
            this._elements[i].Layout(x, y + (buttonHeight * i), width);
        }
    }

    SetVisibility(value) {
        this._scrollView.SetVisibility(value);
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
import UIView from './UIView.js'
import UIGlobals from './UIGlobals.js'
import UIScrollBar from './UIScrollBar.js'

export default class UIScrollView extends UIView {
    container = null; // Phaser container
    horizontalScrollBar = null;
    verticalScrollBar = null;
    showHorizontal = true; // TODO: False by default
    showVertical = true;
    _hidden = false;

    constructor(scene, parent = null) {
        super(scene, parent);

        this.verticalScrollBar = new UIScrollBar(scene);
        this.horizontalScrollBar = new UIScrollBar(scene);
        this.horizontalScrollBar.horizontal = true;
    }

    UpdateColors() {
        this.verticalScrollBar.UpdateColors();
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        const scrollSize = UIGlobals.Sizes.ScrollTrackSize;
        const debugContentRatio = 0.8;

        if (this.showHorizontal && this.showVertical) {
            this.verticalScrollBar.Layout(x + width - scrollSize, y, scrollSize, height - scrollSize, debugContentRatio);
        }
        else if (this.showVertical) {
            this.verticalScrollBar.Layout(x + width - scrollSize, y, scrollSize, height, debugContentRatio);
        }

        if (this.showHorizontal && this.showVertical) {
            this.horizontalScrollBar.Layout(x, y + height - scrollSize, width - scrollSize, scrollSize, debugContentRatio);
        }
        else if (this.showHorizontal) {
            this.horizontalScrollBar.Layout(x, y + height - scrollSize, width, scrollSize, debugContentRatio);
        }

        if (this.showVertical && !this._hidden) {
            this.verticalScrollBar.Show();
        }
        else {
            this.verticalScrollBar.Hide();
        }

        if (this.showHorizontal && !this._hidden) {
            this.horizontalScrollBar.Show();
        }
        else {
            this.horizontalScrollBar.Hide();
        }
    }

    Hide() {
        this._hidden = true;
        
    }

    Show() {
        this._hidden = false;
    }
}
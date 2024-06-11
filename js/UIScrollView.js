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

        this.container = scene.add.container(0, 0);
        this.verticalScrollBar = new UIScrollBar(scene);
        this.horizontalScrollBar = new UIScrollBar(scene);
        this.horizontalScrollBar.horizontal = true;

         // debug only
         this.horizontalScrollBar.current = 0.5;
         this.verticalScrollBar.current = 0.5;
         // end debug

        const self = this;
        const scrollFunction = function(value) {
            self.Layout(self._x, self._y, self._width, self._height);
        }

        this.horizontalScrollBar.onScroll = scrollFunction;
        this.verticalScrollBar.onScroll = scrollFunction;

    }

    UpdateColors() {
        this.verticalScrollBar.UpdateColors();
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        this.container.setPosition(x, y);

        const scrollSize = UIGlobals.Sizes.ScrollTrackSize;
        const bounds = this.container.getBounds();

        let contentRatio = 0.0;
        if (bounds.height > 0) {
            contentRatio = height / bounds.height;
        }

        if (this.showHorizontal && this.showVertical) {
            this.verticalScrollBar.Layout(x + width - scrollSize, y, scrollSize, height - scrollSize, contentRatio);
        }
        else if (this.showVertical) {
            this.verticalScrollBar.Layout(x + width - scrollSize, y, scrollSize, height, contentRatio);
        }

        contentRatio = 0.0;
        if (bounds.width > 0) {
            contentRatio = width / bounds.width;
        }

        if (this.showHorizontal && this.showVertical) {
            this.horizontalScrollBar.Layout(x, y + height - scrollSize, width - scrollSize, scrollSize, contentRatio);
        }
        else if (this.showHorizontal) {
            this.horizontalScrollBar.Layout(x, y + height - scrollSize, width, scrollSize, contentRatio);
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

        let _w = bounds.width - width;
        let _h = bounds.height - height;
        if (this.showVertical) {
            _w += scrollSize;
        }
        if (this.showHorizontal) {
            _h += scrollSize;
        }

        let containerX = x - (_w) * this.horizontalScrollBar.current;
        let containerY = y - (_h) * this.verticalScrollBar.current;
        this.container.setPosition(containerX, containerY);
    }

    Hide() {
        this._hidden = true;
        this.verticalScrollBar.Hide();
        this.horizontalScrollBar.Hide();
        this.container.setActive(false).setVisible(false);
    }

    Show() {
        this._hidden = false;
        if (this.showHorizontal) {
            this.verticalScrollBar.Show();
        }
        if (this.showHorizontal) {
            this.horizontalScrollBar.Show();
        }
        this.container.setActive(true).setVisible(true);
    }
}
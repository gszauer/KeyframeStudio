import UIView from './UIView.js'
import UIGlobals from './UIGlobals.js'
import UIScrollBar from './UIScrollBar.js'

export default class UIScrollView extends UIView {
    container = null; // Phaser container
    horizontalScrollBar = null;
    verticalScrollBar = null;
    showHorizontal = true;
    showVertical = true;
    _hidden = false;
    _maskRect = null;
    _backgroundSprite = null;
    _pin = null;

    constructor(scene, parent = null) {
        super(scene, parent);

        this._pin = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._pin.setOrigin(0, 0);
        this._pin.setDepth(UIGlobals.WidgetLayer); // Nudge a bit

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setOrigin(0, 0);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer - 1); // Nudge a bit

        this.container = scene.add.container(0, 0);
        this.verticalScrollBar = new UIScrollBar(scene);
        this.horizontalScrollBar = new UIScrollBar(scene);
        this.horizontalScrollBar.horizontal = true;

        this.container.add(this._pin);
        this._pin.setPosition(0, 0);
        this._pin.setVisible(false);

        this._maskRect = scene.add.rectangle(0, 0, 100, 100, 0x000000).setVisible(false).setOrigin(0, 0);
        const mask = this._maskRect.createGeometryMask();
        this.container.setMask(mask);
        this.container.setDepth(UIGlobals.WidgetLayer); 

        const self = this;

        const scrollFunction = function(value) {
            self.Layout(self._x, self._y, self._width, self._height);
        }
        this.horizontalScrollBar.onScroll = scrollFunction;
        this.verticalScrollBar.onScroll = scrollFunction;
    }

    ScrollUp() {
        this.verticalScrollBar.current += this.verticalScrollBar.scrollStep;
        if (this.verticalScrollBar.current > 1) {
            this.verticalScrollBar.current = 1;
        }
        this.Layout(this._x, this._y, this._width, this._height);
    }

    ScrollDown() {
        this.verticalScrollBar.current -= this.verticalScrollBar.scrollStep;
        if (this.verticalScrollBar.current < 0) {
            this.verticalScrollBar.current = 0;
        }
        this.Layout(this._x, this._y, this._width, this._height);
    }

    UpdateColors() {
        if (this._backgroundSprite != null) {
            this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
        }
        this.verticalScrollBar.UpdateColors();
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        this.container.setPosition(x, y);

        const scrollSize = UIGlobals.Sizes.ScrollTrackSize;
        const bounds = this.container.getBounds();

        if (this._backgroundSprite != null) {
            this._backgroundSprite.setPosition(x, y);
            this._backgroundSprite.setScale(width, height);
        }

        let maskWidth = width;
        let maskHeight = height;

        let contentRatio = 1.0;
        if (bounds.height > 0) {
            contentRatio = height / bounds.height;
        }

        if (this.showHorizontal && this.showVertical) {
            this.verticalScrollBar.Layout(x + width - scrollSize, y, scrollSize, height - scrollSize, contentRatio);
            maskWidth -= scrollSize;
        }
        else if (this.showVertical) {
            this.verticalScrollBar.Layout(x + width - scrollSize, y, scrollSize, height, contentRatio);
            maskWidth -= scrollSize;
        }

        contentRatio = 1.0;
        if (bounds.width > 0) {
            contentRatio = width / bounds.width;
        }

        if (this.showHorizontal && this.showVertical) {
            this.horizontalScrollBar.Layout(x, y + height - scrollSize, width - scrollSize, scrollSize, contentRatio);
            maskHeight -= scrollSize;
        }
        else if (this.showHorizontal) {
            this.horizontalScrollBar.Layout(x, y + height - scrollSize, width, scrollSize, contentRatio);
            maskHeight -= scrollSize;
        }

        this._maskRect.setPosition(x, y);
        this._maskRect.setSize(maskWidth, maskHeight)

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

        if (containerY > this._y) {
            containerY = this._y;
            this.verticalScrollBar.current = 0;
            if (this.showHorizontal) {
                this.horizontalScrollBar.Layout(this.horizontalScrollBar._x,  this.horizontalScrollBar._y,  this.horizontalScrollBar._width,  this.horizontalScrollBar._height, contentRatio);
            }
        }
        this.container.setPosition(containerX, containerY);
    }

    Hide() {
        this._hidden = true;
        this.verticalScrollBar.Hide();
        this.horizontalScrollBar.Hide();
        this.container.setActive(false).setVisible(false);
        if (this._backgroundSprite != null) {
            this._backgroundSprite.setActive(false).setVisible(false);
        }
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
        if (this._backgroundSprite != null) {
            this._backgroundSprite.setActive(true).setVisible(true);
        }
    }
}
import UIView from './UIView.js'
import UIGlobals from './UIGlobals.js'

export default class UISplitView extends UIView {
    a = null; // Left or top
    b = null; // Right or bottom

    _distance = 0; // In pixels
    horizontal = true;

    // Only one will be true
    pinLeft = true; // If left isn't pinned, right is
    pinTop = true; // If top isn't pinned, bottom is

    // Only set in layout. aSize + seperatorSize + bSize = view size
    _aSize = 0;
    _bSize = 0;

    _dividerSprite = null;

    constructor(scene, parent = null) {
        super(scene, parent);

        this._dividerSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._dividerSprite.setDepth(UIGlobals.WidgetLayer + 1); // Above widgets, below popups
        this._dividerSprite.setOrigin(0, 0);

        this._dividerSprite.setInteractive();
        scene.input.setDraggable(this._dividerSprite);
        const self = this;

        this._dividerSprite.on("pointerover", function (pointer, localX, localY, event) {
            if (UIGlobals.Active == null) {
                UIGlobals.Hot = self._dividerSprite;
            }

            self.UpdateColors();
        });
        this._dividerSprite.on("pointerout", function (pointer, event) {
            if (UIGlobals.Hot == self._dividerSprite) {
                UIGlobals.Hot = null;
            }

            self.UpdateColors();
        });
        

        scene.input.on('dragstart', (pointer, gameObject) => {
            if (gameObject != self._dividerSprite) { return; }
            
            UIGlobals.Active = self._dividerSprite;
        });

        scene.input.on('dragend', (pointer, gameObject) => {
            if (gameObject != self._dividerSprite) { return; }

            if (UIGlobals.Active = self._dividerSprite) {
                UIGlobals.Active = null;
            }
        });

        scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
            if (gameObject != self._dividerSprite) { return; }
            
            if (dragX < self._x) { dragX = self._x; }
            if (dragX > self._x + self._width) { dragX = self._x + self._width; }
            if (dragY < self._y) { dragY = self._y; }
            if (dragY > self._y + self._height) { dragY = self._y + self._height; }

            if (self.horizontal) {
                gameObject.x = dragX;// - Math.floor(UIGlobals.Sizes.SplitViewSplitterSize * 0.5);

                const x = dragX - self._x; // x from 0 to _width
                let t = x / self._width; // x from 0 to 1
                if (!self.pinLeft) {  t = 1.0 - t; }
                self._distance = (self._width * t);
            }
            else {
                gameObject.y = dragY;// - Math.floor(UIGlobals.Sizes.SplitViewSplitterSize * 0.5);

                const y = dragY - self._y; 
                let t = y / self._height;
                if (!self.pinTop) {  t = 1.0 - t; }
                self._distance = (self._height * t);
            }

            self.Layout(self._x, self._y, self._width, self._height);
        });
    }

    Hide() {
        this._dividerSprite.setActive(false).setVisible(false);
        if (this.a != null) {
            this.a.Hide();
        }
        if (this.b != null) {
            this.b.Hide();
        }
    }

    Show() {
        this._dividerSprite.setActive(true).setVisible(true);
        if (this.a != null) {
            this.a.Show();
        }
        if (this.b != null) {
            this.b.Show();
        }
    }

    UpdateColors() {
        if (this.a != null) {
            this.a.UpdateColors();
        }
        if (this.b != null) {
            this.b.UpdateColors();
        }

        let borderTint = UIGlobals.Colors.BorderFraming;//UIGlobals.Colors.ElementBorderTintIdle;

        if (UIGlobals.Hot == this._dividerSprite) {
            borderTint = UIGlobals.Colors.ElementBorderTintIdle;//UIGlobals.Colors.ElementBorderTintHot;
        }
        if (UIGlobals.Active == this._dividerSprite) {
            borderTint = UIGlobals.Colors.ElementBorderTintActive;
        }
        this._dividerSprite.setTint(borderTint);
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        // Calculate aSize and bSize
        const splitterSize = UIGlobals.Sizes.SplitViewSplitterSize;
        this._aSize = this._bSize = this._distance;

        let aX, aY, aWidth, aHeight = 0;
        let bX, bY, bWidth, bHeight = 0;

        if(this.horizontal) {
            aX = x;
            aY = bY = y;
            aHeight = bHeight = height;
            if (this.pinLeft) {
                aWidth = this._distance;
                bWidth = this._bSize = width - splitterSize - this._distance;
            }
            else {
                aWidth = this._aSize = width - splitterSize - this._distance;
                bWidth = this._distance;
            }
            bX = aX + aWidth + splitterSize;

            if (aWidth < 0) { aWidth = 0; }
            this._dividerSprite.setPosition(aX + aWidth, y);
            this._dividerSprite.setScale(splitterSize, height);
        }
        else {
            aY = y;
            aX = bX = x;
            aWidth = bWidth = width;
            if (this.pinTop) {
                aHeight = this._distance;
                bHeight = this._bSize = height - splitterSize - this._distance;
            }
            else {
                aHeight = this._aSize = height - splitterSize - this._distance;
                bHeight = this._distance;
            }
            bY = aY + aHeight + splitterSize;

            if (aHeight < 0) { aHeight = 0; }
            this._dividerSprite.setPosition(x, aY + aHeight);
            this._dividerSprite.setScale(width, splitterSize);
        }
        if (this._aSize < 0) {  this._aSize = 0; }
        if (this._bSize < 0) { this._bSize = 0; }

        if (this.a != null) {
            this.a.Layout(aX, aY, aWidth, aHeight);
        }

        if (this.b != null) {
            this.b.Layout(bX, bY, bWidth, bHeight);
        }

        this.UpdateColors();
    }
}
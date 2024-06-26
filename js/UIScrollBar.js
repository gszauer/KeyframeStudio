import UIGlobals from './UIGlobals.js'

// https://csdgn.org/article/scrollbar
export default class UIScrollBar {
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    _trackX = 0;
    _trackY = 0;
    _trackWidth = 0;
    _trackHeight = 0;

    _gripX = 0;
    _gripY = 0;
    _gripWidth = 0;
    _gripHeight = 0;
    _contentRatio = 0;

    horizontal = false;
    onScroll = null; // OnScroll(float normalized);
    current = 0; // 0 to 1 depending on current scroll

    _trackSprite = null;
    _gripSprite = null;

    get scrollStep() {
        return 0.1;
    }

    constructor(scene) {
        const self = this;

        this._trackSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._trackSprite.setOrigin(0, 0);
        this._trackSprite.setDepth(UIGlobals.WidgetLayer);

        this._gripSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._gripSprite.setOrigin(0, 0);
        this._gripSprite.setDepth(UIGlobals.WidgetLayer);

        this._gripSprite.setInteractive();
        scene.input.setDraggable(this._gripSprite);

        self._gripSprite.on("pointerover", function (pointer, localX, localY, event) {
            if (UIGlobals.Active == null) {
                UIGlobals.Hot = self._gripSprite;
            }

            self.UpdateColors();
        });
        self._gripSprite.on("pointerout", function (pointer, event) {
            if (UIGlobals.Hot == self._gripSprite) {
                UIGlobals.Hot = null;
            }

            self.UpdateColors();
        });

        scene.input.on('dragstart', (pointer, gameObject) => {
            if (gameObject != self._gripSprite) { return; }
            
            UIGlobals.Active = self._gripSprite;
            self.UpdateColors();
        });

        scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
            if (gameObject != self._gripSprite) { return; }
            UIGlobals.Active = self._gripSprite;

            let x = gameObject.x;
            let y = gameObject.y;
            if (self.horizontal) {
                let x = dragX;//pointer.x;
                let trackWidth = self._trackWidth - self._gripWidth;
                if (trackWidth < 0) { trackWidth = 0; }
                if (x < self._trackX) { x = self._trackX; }
                if (x > self._trackX + trackWidth) { x = self._trackX + trackWidth; }

                if (trackWidth <= 0) {
                    self.current = 0;
                }
                else {
                    self.current = (x - self._trackX) / trackWidth;
                }
                
            }
            else {
                let y = dragY;//pointer.y;
                let trackHeight = self._trackHeight - self._gripHeight;
                if (trackHeight < 0) { trackHeight = 0; }
                if (y < self._trackY) { y = self._trackY; }
                if (y > self._trackY + trackHeight) { y = self._trackY + trackHeight; }

                if (trackHeight <= 0) {
                    self.current = 0;
                }
                else {
                    self.current = (y - self._trackY) / trackHeight;
                }
            }
            gameObject.setPosition(x, y);
            if (self.onScroll != null) {
                self.onScroll(self.current);
            }
            else {
                self.Layout(self._x, self._y, self._width, self._height, self._contentRatio);
            }
        });

        scene.input.on('dragend', (pointer, gameObject) => {
            if (gameObject != self._gripSprite) { return; }

            if (UIGlobals.Active == self._gripSprite) {
                UIGlobals.Active = null;
            }
            self.UpdateColors();

        });
    }

    Hide() {
        this._trackSprite.setActive(false).setVisible(false);
        this._gripSprite.setActive(false).setVisible(false);
    }

    Show() {
        this._trackSprite.setActive(true).setVisible(true);
        this._gripSprite.setActive(true).setVisible(true);
    }

    UpdateColors() {
        this._trackSprite.setTint(UIGlobals.Colors.BackgroundLayer0);
        let gripColor = UIGlobals.Colors.BackgroundLayer2;

        if (UIGlobals.Hot == this._gripSprite) {
            gripColor = UIGlobals.Colors.BorderDecorative;
        }

        if (UIGlobals.Active == this._gripSprite) {
            gripColor = UIGlobals.Colors.BorderFraming;
        }

        if (this._contentRatio == 1) {
            gripColor = UIGlobals.Colors.BackgroundLayer0;
        }

        this._gripSprite.setTint(gripColor);
    }
    
    Layout(x, y, width, height, contentRatio) {
        if (contentRatio === null || contentRatio === undefined) {
            throw Error("cotnent ratio is not optional for UIScrollBar");
        }

        const scrollTrackSize = UIGlobals.Sizes.ScrollTrackSize;
        const horizontal = this.horizontal;
        const minGripSize = UIGlobals.Sizes.ScrollTrackMinGripSize;
        let borderX = UIGlobals.Sizes.ScrollBorderSize;
        let borderY = UIGlobals.Sizes.ScrollBorderSize;

        if (contentRatio < 0) { contentRatio = 0; }
        if (contentRatio > 1) { contentRatio = 1; }
        if (this.current < 0) { this.current = 0; }
        if (this.current > 1) { this.current = 1; }

        this._contentRatio = contentRatio;

        if (horizontal) {
            height = scrollTrackSize;
            if (width < scrollTrackSize * 4) {
                width = scrollTrackSize * 4;
            }
            borderX = 0;
        }
        else {
            width = scrollTrackSize;
            if (height < scrollTrackSize * 4) {
                height = scrollTrackSize * 4;
            }
            borderY = 0;
        }

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._trackX = x;
        this._trackY = y;
        this._trackWidth = width;
        this._trackHeight = height;

        this._trackSprite.setPosition(x, y);
        this._trackSprite.setScale(width, height);

        x += borderX;
        y += borderY;
        width -= borderX * 2;
        height -= borderY * 2;

        let gripWidth = scrollTrackSize - borderX * 2;
        let gripHeight = scrollTrackSize - borderY * 2;

        if (horizontal) {
            gripWidth = width * contentRatio;
            if (gripWidth < minGripSize) {
                gripWidth = minGripSize;
            }
        }
        else {
            gripHeight = height * contentRatio;
            if (gripHeight < minGripSize) {
                gripHeight = minGripSize;
            }
        }

        this._gripX = x;
        this._gripY = y;
        this._gripWidth = gripWidth;
        this._gripHeight = gripHeight;

        let _x = x;
        let _y = y;

        if (horizontal) {
            _x = x + (width - gripWidth) * this.current;
        }
        else {
            _y = y + (height - gripHeight) * this.current;
        }

        this._gripSprite.setPosition(_x, _y);
        this._gripSprite.setScale(gripWidth, gripHeight);

        this.UpdateColors();
    }
}
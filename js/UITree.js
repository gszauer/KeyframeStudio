import UIGlobals from './UIGlobals.js'
import UIScrollView from './UIScrollView.js'
import UITreeNode from './UITreeNode.js'

export default class UITree {
    _scene = null;
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    _scrollView = null;
    _inputItem = null;
    _roots = [];

    _hoverIndex = -1;

    constructor(scene) {
        this._scene = scene;
        const self = this;

        this._scrollView = new UIScrollView(scene, this);
    
        this._inputItem = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._inputItem.setDepth(UIGlobals.WidgetLayer - 2);
        this._inputItem.setOrigin(0, 0);
        this._inputItem.setInteractive();
        scene.input.setDraggable(this._inputItem);

        { // Pointer events
            this._inputItem.on("pointerover", function (pointer, localX, localY, event) {
                self._hoverIndex = -1;
                self.UpdateColors();
            });
            this._inputItem.on("pointermove", function (pointer, localX, localY, event) {
                pointer = self.ConstrainPointer(pointer);

                let y = pointer.y - self._scrollView._y;
                y -= self._scrollView.container.y - self._scrollView._y;

                let selectionIndex = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);
                if (selectionIndex < 0 || selectionIndex >= self._numButtons) {
                    selectionIndex = -1;
                }
                self._hoverIndex = selectionIndex;

                self.UpdateColors();
            });
            this._inputItem.on("pointerout", function (pointer, event) {
                self._hoverIndex = -1;
                self.UpdateColors();
            });
        }
    }

    ConstrainPointer(pointer) {
        const result = {
            x: pointer.x, 
            y: pointer.y
        };

        if (result.y < this._scrollView._maskRect.y) { 
            result.y = this._scrollView._maskRect.y 
        }
        if (result.y > this._scrollView._maskRect.y + this._scrollView._maskRect.height) { 
            result.y = this._scrollView._maskRect.y + this._scrollView._maskRect.height; 
        }

        return result;
    }

    UpdateColors() {
        this._scrollView.UpdateColors();

        let current = 0;
        const colors = [
            UIGlobals.Colors.BackgroundLayer1,
            UIGlobals.Colors.BackgroundLayer1AndAHalf
        ];
        const highlights = [
            UIGlobals.Colors.Dark.Blue300,
            UIGlobals.Colors.Dark.Blue200,
         ];
        const roots = this._roots;
        const length = this._roots.length;
        const text = UIGlobals.Colors.Text;
        
        for (let i = 0; i < length; ++i) {
            roots[i].ForEach((node, depth) => {
                let color = colors[(current) % 2];
                if (this._hoverIndex == current && UIGlobals.Active == null) {
                    color = highlights[(current) % 2];
                }
                node._SetColor(color, text);
                current += 1;
            });
        }
    }

    _AddToRoots(element) {
        const length = this._roots.length;
        for (let i = 0; i < length; ++i) {
            if (this._roots[i] == element) {
                throw new Error("Duplicate root found");
            }
        }
        this._roots.push(element);
    }

    _RemoveFromRoots(element) {
        let toRemove = -1;
        const length = this._roots.length;
        for (let i = 0; i < length; ++i) {
            if (this._roots[i] == element) {
                toRemove = i;
                break;
            }
        }

        if (toRemove != -1) {
            this._roots.splice(toRemove, 1);
        }
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._inputItem.setPosition(x, y);
        this._inputItem.setScale(width, height);

        // Lay out tree nodes
        x = 0;
        y = 0;
        //width = this._scrollView._maskRect.width;
        height = UIGlobals.Sizes.TreeItemHeight;

        const scrollSize = UIGlobals.Sizes.ScrollTrackSize;
        if (this._scrollView.showHorizontal) {
            width -= scrollSize;
        }
        else if (this._scrollView.showVertical) {
            height -= scrollSize;
        }

        const roots = this._roots;
        const length = this._roots.length;
        for (let i = 0; i < length; ++i) {
            roots[i].ForEach((node, depth) => {
                node.Layout(x, y, width, height, depth);
                y += height;
            });
        }

        this._scrollView.Layout(this._x, this._y, this._width, this._height);

        this.UpdateColors();
    }

    SetVisibility(value) {
        this._scrollView.SetVisibility(value);
        this._inputItem.setActive(value).setVisible(value);
    }

    Add(name, userData = null) {
        const node = new UITreeNode(this, name, null);
        node._userData = userData;
        node._AddToContainer(this._scrollView.container);
        this._roots.push(node);
        return node;
    }
}
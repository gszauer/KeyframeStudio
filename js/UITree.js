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

    constructor(scene) {
        this._scene = scene;

        this._scrollView = new UIScrollView(scene, this);
    
        this._inputItem = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._inputItem.setDepth(UIGlobals.WidgetLayer - 2);
        this._inputItem.setOrigin(0, 0);
        this._inputItem.setInteractive();
        scene.input.setDraggable(this._inputItem);
    }

    UpdateColors() {
        this._scrollView.UpdateColors();

        let current = 0;
        const colors = [
            0xff0000,
            0x00ff00
        ];

        const roots = this._roots;
        const length = this._roots.length;
        
        for (let i = 0; i < length; ++i) {
            roots[i].ForEach((node, depth) => {
                node._SetColor(colors[current++ % 2 == 0? true : false]);
            });
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

        this._scrollView.Layout(x, y, width, height);

        // Lay out tree nodes
        x = 0;
        y = 0;
        width = 200; // TODO: Figure this out
        height = UIGlobals.Sizes.TreeItemHeight;

        const roots = this._roots;
        const length = this._roots.length;
        for (let i = 0; i < length; ++i) {
            roots[i].ForEach((node, depth) => {
                node.Layout(x, y, width, height);
                y += height;
            });
        }
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
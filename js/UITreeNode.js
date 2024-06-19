import UIGlobals from './UIGlobals.js'
import UITree from './UITree.js'

export default class UITreeNode {
    _tree = null;
    _name = null;

    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    _parent = null;
    _firstChild = null;
    _nextSibling = null;
    // Maybe store depth? Or calculate it as we go about??

    _background = null;
    _label = null;
    _userData = null;

    constructor(tree, name = "New Node", parent = null) {
        this._tree = tree;
        this._name = name;
        
        if (parent != null) {
            parent.AddChild(this);
        }

        const scene = tree._scene;

        this._background = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._background.setDepth(UIGlobals.WidgetLayer);
        this._background.setOrigin(0, 0);

        this._label = scene.add.bitmapText(0, 0, UIGlobals.Font200, name);
        this._label.setDepth(UIGlobals.WidgetLayer);
        this._label.setOrigin(0, 0);
        this._label.text = name;
    }

    _SetColor(background = 0x000000, text = 0xffffff) {
        this._background.setTint(background);
        this._label.setTint(text);
    }

    _AddToContainer(container) {
        container.add(this._background);
        container.add(this._label);
    }

    _RemoveChild(child) {
        throw new Error("Not implemented exception");
        // Should unlink child
        // And add child to the end of the roots array in tree
    }

    SetParent(newParent) {
        if (this._parent != null) {
            this._parent._RemoveChild(this);
        }

        if (newParent != null) {
            newParent.AddChild(this);
        }
    }

    AddChild(child) {
        // Unlink child from old tree if applicable
        if (child.parent != null) {
            child.parent._RemoveChild(child);
        }

        // Set the parent of the node
        child.parent = this;

        // Add to the end of the tree list
        if (this._firstChild == null) {
            this._firstChild = child;
        }
        else {
            let iter = this._firstChild;
            while(iter._nextSibling != null) {
                iter = iter._nextSibling;
            }
            iter._nextSibling = child;
        }

        if (child._nextSibling != null) {
            throw new Error("Expected next sibling to be null on add");
        }
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0;}

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._background.setPosition(x, y);
        this._background.setScale(width, height);
    }

    ForEach(callback) {
        // const root = this
        let itr = this;
        let traversing = true;
        let depth = 0;
        while (traversing) {
            callback(itr, depth);
     
            if (itr._firstChild) {
                itr = itr._firstChild;
                depth += 1;
            }
            else {
                while (itr._nextSibling == null) {
                    if (itr == this) {
                        traversing = false;
                        break;
                    }
                    itr = itr._parent;
                    depth -= 1;
                }
                if (itr == this) { // Prevent stepping to the roots sibling
                    traversing = false;
                    break;
                }
                itr = itr._nextSibling;
                // No change to depth
            }
        }
    }
}
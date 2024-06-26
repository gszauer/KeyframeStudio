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

    get indentLevel() {
        let result = 0;
        for (let iter = this._parent; iter != null; iter = iter._parent) {
            result += 1;
        }
        return result;
    }

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
        // Unlinks child, and add child to the end of the roots array in tree

        if (child == null) {
            throw new Error("Not sure if this is valid");
        }
        if (this._firstChild == null) {
            throw new Error("There is no children to remove");
        }

        if (child == this._firstChild) {
            this._firstChild = this._firstChild._nextSibling;
        }
        else {
            let prev = this._firstChild;
            let cur = this._firstChild._nextSibling;
            while (cur != null) {
                if (cur == child) {
                    prev._nextSibling = cur._nextSibling;
                    break;
                }
                cur = cur._nextSibling;
                prev = prev._nextSibling;
            }
        }

        child._nextSibling = null;
        child._parent = null;
        this._tree._AddToRoots(child);
    }

    SetParent(newParent) {
        if (this._parent != null) {
            this._parent._RemoveChild(this);
        }
        this._tree._RemoveFromRoots(this);

        if (newParent != null) {
            newParent.AddChild(this);
        }
        else {
            this._parent = null;
            this._tree._AddToRoots(this);
        }

        return this;
    }

    _AddCommon(child, message = "undefined") {
        // Unlink child from old tree if applicable
        if (child._parent != null) {
            child._parent._RemoveChild(child);
        }
        this._tree._RemoveFromRoots(child);

        // Set the parent of the node
        if (child._nextSibling != null) {
            throw new Error("Expected next sibling to be null in: " + message);
        }
    }

    AddChild(child) {
        this._AddCommon(child, "AddChild");
        child._parent = this;

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
    }

    AddChildFront(child) {
        this._AddCommon(child, "AddChildFront");
        child._parent = this;

        // Add to the front of the tree list
        child._nextSibling = this._firstChild;
        this._firstChild = child;
    }

    AddChildAfter(newChild, addAfterThisChild) {
        if (newChild == addAfterThisChild) {
            return;
        }
        
        this._AddCommon(newChild, "AddChildAfter");
        newChild._parent = this;

        // Add to the end of the tree list
        if (this._firstChild == null) {
            this._firstChild = newChild;
        }
        else {
            let found = false;
            for (let iter = this._firstChild; iter != null; iter = iter._nextSibling) {
                if (iter == addAfterThisChild) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw new Error("AddChildAfter second argument is not a valid child");
            }

            newChild._nextSibling = addAfterThisChild._nextSibling;
            addAfterThisChild._nextSibling = newChild;
        }
    }

    Layout(x, y, width, height, depth = 0) {
        if (width < 0) { width = 0; }
        height = UIGlobals.Sizes.TreeItemHeight;

        const marginLeft = UIGlobals.Sizes.TreeItemMarginLeft;
        const indent = UIGlobals.Sizes.TreeItemIndent;

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._background.setPosition(x, y);
        this._background.setScale(width, height);

        this._label.setPosition(x + marginLeft + indent * depth, y);

    }

    ForEach(callback) {
        // const root = this
        //const oldParent = this._parent;
        //this._parent = null;

        const root = this;
        let itr = root;
        let traversing = true;
        let depth = 0;

        while (traversing) {
            //if (itr == this) { this._parent = oldParent; }
            callback(itr, depth);
            //if (itr == this) { this._parent = null; }
     
            if (itr._firstChild != null) {
                itr = itr._firstChild;
                depth += 1;
            }
            else {
                while (itr._nextSibling == null) {
                    if (itr == root) {
                        traversing = false;
                        break;
                    }
                    itr = itr._parent;
                    depth -= 1;
                }
                if (itr == root) { // Prevent stepping to the roots sibling
                    traversing = false;
                    break;
                }
                itr = itr._nextSibling;
                // No change to depth
            }
        }

        //this._parent = oldParent;
    }

    get name() {
        return this._name;
    }

    get parent() {
        return this._parent;
    }
}
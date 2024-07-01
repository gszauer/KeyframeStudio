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
    _pressedIndex = -1;

    _isDragging = false;
    _dragStartIndex = -1;
    _selectedIndex = -1;
    
    _orderIndicator = null; // Drag graphics
    _numButtons = 0;

    onSelected = null; //onSelected(UITreeNode node);
    onRearranged = null; // onRearranged(UITreeNode target);

    constructor(scene) {
        this._scene = scene;
        const self = this;

        const scrollView = this._scrollView = new UIScrollView(scene, this);
    
        this._inputItem = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._inputItem.setDepth(UIGlobals.WidgetLayer - 2);
        this._inputItem.setOrigin(0, 0);
        this._inputItem.setInteractive();
        scene.input.setDraggable(this._inputItem);

        this._orderIndicator = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._orderIndicator.setDepth(UIGlobals.WidgetLayer + 2);
        this._orderIndicator.setOrigin(0, 0.5);
        this._orderIndicator.setActive(false).setVisible(false);

        // Helper functions
        const ConstrainPointer = function(pointer) {
            const result = {
                x: pointer.x, 
                y: pointer.y
            };
    
            if (result.y < scrollView._maskRect.y) { 
                result.y = scrollView._maskRect.y 
            }
            if (result.y > scrollView._maskRect.y + scrollView._maskRect.height) { 
                result.y = scrollView._maskRect.y + scrollView._maskRect.height; 
            }
    
            return result;
        };
        const GetNodeByIndex = function(index) {
            const roots = self._roots;
            const length = roots.length;
            let idx = 0;
            let result = null;
            for (let i = 0; i < length; ++i) {
                roots[i].ForEach((node, depth) => {
                    if (idx == index) {
                        result = node;
                    }
                    idx += 1;
                });
                if (result != null) {
                    break;
                }
            }
            return result;
        }
        const GetIndexByNode = function(_node) {
            const roots = self._roots;
            const length = roots.length;
            let idx = 0;
            let result = -1;
            for (let i = 0; i < length; ++i) {
                roots[i].ForEach((node, depth) => {
                    if (node == _node) {
                        result = idx;
                    }
                    idx += 1;
                });
                if (result != -1) {
                    break;
                }
            }
            return result;
        }
        const GetDepthByIndex = function(index) {
            const roots = self._roots;
            const length = roots.length;
            let idx = 0;
            let result = -1;
            for (let i = 0; i < length; ++i) {
                roots[i].ForEach((node, depth) => {
                    if (idx == index) {
                        result = depth;
                    }
                    idx += 1;
                });
                if (result != -1) {
                    break;
                }
            }
            return result;
        }
        const GetNodeIndexUnderMouse = function(pointer) { 
            let y = pointer.y - self._scrollView._y;
            y -= self._scrollView.container.y - self._scrollView._y;

            let selectionIndex = Math.floor(y / UIGlobals.Sizes.TreeItemHeight);
            if (selectionIndex < 0 || selectionIndex >= self._numButtons) {
                selectionIndex = -1;
            }
            return selectionIndex;
        }
        const UpdateDragIndicator = function(pointer) {
            let y = pointer.y - self._scrollView._y;
            y -= self._scrollView.container.y - self._scrollView._y;
            
            const deltaY = self._scrollView.container.y - self._scrollView._y;

            let selectionIndex = Math.floor(y / UIGlobals.Sizes.TreeItemHeight);
            let selectionPosition = 0; // -1, 0, 1
            let index = selectionIndex;

            if (selectionIndex < 0 || selectionIndex >= self._numButtons) {
                selectionIndex = -1;
                index = -1;
            }
            else {
                const quarter = Math.floor(UIGlobals.Sizes.TreeItemHeight * 0.25);
                if (quarter < 5) { quarter = 5; }

                let top = selectionIndex * UIGlobals.Sizes.TreeItemHeight;
                let bottom = top + quarter;
                if (y >= top && y <= bottom) {
                    selectionPosition = -1;
                }
                else {
                    index += 1;
                }

                top = (selectionIndex * UIGlobals.Sizes.TreeItemHeight) + UIGlobals.Sizes.TreeItemHeight - quarter;
                bottom = top + quarter;
                if (y >= top && y <= bottom) {
                    selectionPosition = 1;
                }
            }
            
            let itemWorldY = self._scrollView._y + deltaY + (index * UIGlobals.Sizes.TreeItemHeight);

            if (selectionIndex >= 0 && selectionIndex <= self._numButtons) {
                let x = self._x;
                let width = self._scrollView._maskRect.width;
                
                let indent = UIGlobals.Sizes.TreeItemIndent * GetDepthByIndex(selectionIndex);
                if (selectionPosition == 1) {
                    indent = UIGlobals.Sizes.TreeItemIndent * (GetDepthByIndex(selectionIndex) + 1);
                }
                x += indent;
                width -= indent;

                self._orderIndicator.setPosition(x, itemWorldY);
                self._orderIndicator.setScale(width, UIGlobals.Sizes.ListBoxOrderIndicator);
                return true;
            }

            return false;
        }
        const MouseUp = function(pointer) {
            let index = -1;
            if (UIGlobals.Active != null && UIGlobals.Active == self._inputItem) {
                let left = self._inputItem.x;
                let right = left + self._inputItem.scaleX;
                let top = self._inputItem.y;
                let bottom = top + self._inputItem.scaleY;

                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    index = GetNodeIndexUnderMouse(pointer);
                    
                    if (index == self._pressedIndex && index >= 0) {
                        self._selectedIndex = index;
                        if (self.onSelected != null) {
                            self.onSelected(GetNodeByIndex(index));
                        }
                    }
                }
                
                self._pressedIndex = -1;
                UIGlobals.Active = null;
                self.UpdateColors();
            }
            return index;
        }
        const CanReparent = function(child, parent) {
            if (child == parent) {
                return false;
            }
            if (child == null) {
                return false;
            }
            for(let iter = parent; iter != null; iter = iter._parent) {
                if (iter == child) {
                    return false;
                }
            }
            return true;
        }
        

        { // Pointer events
            this._inputItem.on("pointerover", function (pointer, localX, localY, event) {
                self._hoverIndex = -1;
                self.UpdateColors();
            });
            this._inputItem.on("pointermove", function (pointer, localX, localY, event) {
                pointer = ConstrainPointer(pointer);
                self._hoverIndex = GetNodeIndexUnderMouse(pointer);
                self.UpdateColors();
            });
            this._inputItem.on("pointerout", function (pointer, event) {
                self._hoverIndex = -1;
                self.UpdateColors();
            });
            this._inputItem.on('wheel', (pointer, deltaX, deltaY, deltaZ, event) => {
                if (deltaY > 0) { self._scrollView.ScrollUp(); }
                else if (deltaY < 0) { self._scrollView.ScrollDown(); }
            });
        }
        { // Drag events
            // Drag start / mouse down
            scene.input.on('dragstart', (pointer, gameObject) => {
                if (gameObject != self._inputItem) { return; }
                pointer = ConstrainPointer(pointer);

                UIGlobals.Active = self._inputItem;
                self._pressedIndex = self._selectedIndex =  GetNodeIndexUnderMouse(pointer);
                
                if (self.onSelected != null) {
                    const node = GetNodeByIndex(self._pressedIndex);
                    self.onSelected(node);
                }
    
                self.UpdateColors();
            });
            scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
                if (gameObject != self._inputItem) { return; }
                pointer = ConstrainPointer(pointer);

                UIGlobals.Active = self._inputItem;

                if (!self._isDragging) { // First time moved
                    self._dragStartIndex = GetNodeIndexUnderMouse(pointer);
                    self._isDragging = true;
                    
                    self._orderIndicator.setActive(true).setVisible(true);
                    self._orderIndicator.setTint(UIGlobals.Colors.Dark.Blue300);

                    if (self.onSelected != null) {
                        const node = GetNodeByIndex(self._dragStartIndex);
                        self.onSelected(node);
                    }

                    self.UpdateColors();
                }

                UpdateDragIndicator(pointer);
            });
            // Drag end / mouse up
            scene.input.on('dragend', (pointer, gameObject) => {
                if (gameObject != self._inputItem) { return; }
                pointer = ConstrainPointer(pointer);

                const wasDragging = self._isDragging;
                const dragStartIndex = self._dragStartIndex;
                
                self._isDragging = false;
                self._dragStartIndex = -1;
                self._hoverIndex = -1; // Not sure about this

                // Mouse up:
                if (MouseUp(pointer) < 0 && !wasDragging) {
                    // MouseUp sets pressed index and active
                    self._selectedIndex = -1;
                    if (self.onSelected != null) {
                        self.onSelected(null);
                    }
                }

                if (wasDragging) { 
                    const pointerY = (pointer.y - self._scrollView._y) -
                                     (self._scrollView.container.y - self._scrollView._y);
        
                    let selectionIndex = GetNodeIndexUnderMouse(pointer);
                    let dragStopIndex = selectionIndex;
        
                    if (selectionIndex < 0) {
                        selectionIndex = -1;
                        dragStopIndex = -1;
                    }
                    else if (selectionIndex > self._numButtons && self._numButtons > 0) {
                        selectionIndex = -1;
                        dragStopIndex = self._numButtons;
                    }
                    else {
                        const quarter = Math.floor(UIGlobals.Sizes.TreeItemHeight * 0.25);
                        if (quarter < 5) { quarter = 5; }
        
                        let top = selectionIndex * UIGlobals.Sizes.TreeItemHeight;
                        let bottom = top + quarter;

                        let reLayout = false;
                        
                        // In the top third!
                        if (pointerY >= top && pointerY <= bottom) {
                            const topIndex = selectionIndex - 1;

                            if (topIndex < 0) { // Insert head
                                const newRoot = GetNodeByIndex(dragStartIndex);
                                if (newRoot.parent != null) {
                                    newRoot.SetParent(null);
                                }
                                self._RemoveFromRoots(newRoot);
                                self._AddToRootsFront(newRoot);
                                reLayout = true;
                                self._selectedIndex = 0;
                            }
                            else { // Insert above node
                                const topNode = GetNodeByIndex(topIndex);
                                const newChildNode = GetNodeByIndex(dragStartIndex);
                                let newParentNode = null;

                                if (topNode.parent != null) {
                                    newParentNode = topNode.parent; 

                                    if (CanReparent(newChildNode, newParentNode)) {
                                        newParentNode.AddChildAfter(newChildNode, topNode);
                                        reLayout = true;
                                        self._selectedIndex = GetIndexByNode(newChildNode);
                                    }
                                }
                                else {
                                    const newRoot = GetNodeByIndex(dragStartIndex);
                                    if (newRoot == null) {
                                        console.error("Debugging, stop here");
                                    }
                                    if (newRoot.parent != null) {
                                        newRoot.SetParent(null);
                                    }
                                    self._RemoveFromRoots(newRoot);
                                    self._AddToRootsAfter(newRoot, topNode);
                                    reLayout = true;
                                    self._selectedIndex = GetIndexByNode(newRoot);
                                }
                            }
                        }
                        else {
                            top = (selectionIndex * UIGlobals.Sizes.TreeItemHeight) + UIGlobals.Sizes.TreeItemHeight - quarter;
                            bottom = top + quarter;

                            // In the bottom third
                            if (pointerY >= top && pointerY <= bottom) { // Bottom Logic
                                const newParentNode = GetNodeByIndex(dragStopIndex);
                                const newChildNode = GetNodeByIndex(dragStartIndex);
                                if (CanReparent(newChildNode, newParentNode)) {
                                    newParentNode.AddChildFront(newChildNode);
                                    reLayout = true;
                                    self._selectedIndex = GetIndexByNode(newChildNode);
                                }
                            }
                            // In the middle
                            else {
                                top = selectionIndex * UIGlobals.Sizes.TreeItemHeight;
                                bottom = top + UIGlobals.Sizes.TreeItemHeight;
                                
                                if (pointerY >= top && pointerY <= bottom) { // Middle Logic
                                    const newChildNode = GetNodeByIndex(dragStartIndex);
                                    const insertAfter = GetNodeByIndex(dragStopIndex);

                                    if (insertAfter != null) {
                                        if (insertAfter._parent != null) {
                                            insertAfter._parent.AddChildAfter(newChildNode, insertAfter);
                                            reLayout = true;
                                            self._selectedIndex = GetIndexByNode(newChildNode);
                                        }
                                        else {
                                            if (newChildNode == null) {
                                                console.error("Going to crash, newChildNode should not be null!");
                                            }
                                            if (newChildNode.parent != null) {
                                                newChildNode.SetParent(null);
                                            }
                                            self._RemoveFromRoots(newChildNode);
                                            self._AddToRootsAfter(newChildNode, insertAfter);
                                            reLayout = true;
                                            self._selectedIndex = GetIndexByNode(newChildNode);
                                        }
                                    }
                                }
                            }
                        }

                        if (reLayout) {
                            if (self.onRearranged != null) {
                                let callbackNode = GetNodeByIndex(self._selectedIndex);
                                self.onRearranged(callbackNode);
                            }
                            self.Layout(self._x, self._y, self._width, self._height);
                        }
                    }

                    if (dragStartIndex != -1) {
                        let dragDelta = dragStopIndex - dragStartIndex;

                        if (dragDelta > 0) {
                            dragDelta -= 1;
                            dragStopIndex -= 1;
                        }

                        if (dragDelta != 0) {
                            if (self.onSelected != null) {
                                self.onSelected(GetNodeByIndex(self._selectedIndex));
                            }
                            self.UpdateColors();
                        }
                    }
                }
            });
        }
    }

    Remove(node) {
        if (node._parent != null) {
            node._parent._RemoveChild(node);
        }
        this._RemoveFromRoots(node);
        // The node should be orphaned at this point
        this._UpdateNumButtons();
        this.Layout();
        this.UpdateColors();
        node.Destroy();
    }

    Deselect() {
        if (this._selectedIndex == -1) {
            return;
        }

        this._selectedIndex = -1;
        if (this.onSelected != null) {
            this.onSelected(null);
        }
        this.UpdateColors();
    }

    UpdateColors() {
        this._scrollView.UpdateColors();

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

        const selectionIndex = this._selectedIndex;
        const pressedIndex = this._pressedIndex;
        const hoverIndex = this._hoverIndex;

        let current = 0;
        for (let i = 0; i < length; ++i) {
            roots[i].ForEach((node, depth) => {
                let color = colors[(current) % 2];
                if (selectionIndex == current || pressedIndex == current) {
                    color = UIGlobals.Colors.Dark.Blue100;
                }
                else if (hoverIndex == current && UIGlobals.Active == null) {
                    color = highlights[(current) % 2];
                }
                node._SetColor(color, text);
                current += 1;
            });
        }
        this._orderIndicator.setVisible(this._isDragging);
    }

    static ArrayMove = function(arr, old_index, new_index) {
        if (new_index >= arr.length) {
            var k = new_index - arr.length + 1;
            while (k--) {
                arr.push(undefined);
            }
        }
        arr.splice(new_index, 0, arr.splice(old_index, 1)[0]);
        return arr; 
    };

    _AddToRoots(element) {
        if (element._parent != null) {
            throw new Error("Can't add parented element to roots");
        }
        const length = this._roots.length;
        for (let i = 0; i < length; ++i) {
            if (this._roots[i] == element) {
                throw new Error("Duplicate root found");
            }
        }
        this._roots.push(element);

        element._parent = null;
        element._nextSibling = null;

        this._UpdateNumButtons();

    }

    _AddToRootsAfter(newElement, addAfterThis) {
        this._AddToRoots(newElement);

        let newIndex = -1;
        let oldIndex = -1

        const length = this._roots.length;
        for (let i = 0; i < length; ++i) {
            if (this._roots[i] == addAfterThis) {
                newIndex = i + 1;
            }
            else if (this._roots[i] == newElement) {
                oldIndex = i;
            }
        }

        if (newIndex != -1 && oldIndex != -1 && newIndex < length) {
            UITree.ArrayMove(this._roots, oldIndex, newIndex);
        }

        this._UpdateNumButtons();
    }

    _AddToRootsFront(element) {
        this._AddToRoots(element);
        UITree.ArrayMove(this._roots, this._roots.length - 1, 0);
        this._UpdateNumButtons();
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

        element._parent = null;
        element._nextSibling = null;

        this._UpdateNumButtons();
    }

    Layout(x, y, width, height) {
        if (x === undefined && y === undefined && width === undefined && height === undefined) {
            x = this._x;
            y = this._y;
            width = this._width;
            height = this._height;
        }
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

        const horizontalScrollBar = this._scrollView.horizontalScrollBar;

        const roots = this._roots;
        const length = this._roots.length;
        let right = this._width;

        for (let i = 0; i < length; ++i) {
            roots[i].ForEach((node, depth) => {
                node.Layout(x, y, width, height, depth);
                if (node._label.x + node._label.width > right) {
                    right = node._label.x + node._label.width;
                }
                y += height;
            });
        }

        // Make sure BG graphics are wide enough
        if (right != this._width) {
            const itemHeight = UIGlobals.Sizes.TreeItemHeight;
            for (let i = 0; i < length; ++i) {
                roots[i].ForEach((node, depth) => {
                    node._background.setScale(right, itemHeight);
                });
            }
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
        this._UpdateNumButtons();
        return node;
    }

    _UpdateNumButtons() {
        const roots = this._roots;
        const length = roots.length;
        let total = 0;
        for (let i = 0; i < length; ++i) {
            roots[i].ForEach((node, depth) => {
                total += 1;
            });
        }
        this._numButtons = total;
        return total;
    }
}
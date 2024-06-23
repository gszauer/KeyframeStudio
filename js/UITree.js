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
        const UpdateDragIndicator = function(pointer) {
            pointer = ConstrainPointer(pointer);

            let y = (pointer.y + UIGlobals.Sizes.TreeItemHeight * 0.5) - self._scrollView._y;
            const deltaY = self._scrollView.container.y - self._scrollView._y;
            y -= deltaY;
            const index = Math.floor(y / UIGlobals.Sizes.TreeItemHeight);
            let itemWorldY = self._scrollView._y + deltaY + (index * UIGlobals.Sizes.TreeItemHeight);

            if (index >= 0 && index <= self._numButtons) {
                self._orderIndicator.setPosition(self._x, itemWorldY);
                self._orderIndicator.setScale(self._scrollView._maskRect.width, UIGlobals.Sizes.ListBoxOrderIndicator);
                return true;
            }

            return false;
        }
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
        const mouseRelease = function(pointer) {
            let index = -1;
            if (UIGlobals.Active != null && UIGlobals.Active == self._inputItem) {
                let left = self._inputItem.x;
                let right = left + self._inputItem.scaleX;
                let top = self._inputItem.y;
                let bottom = top + self._inputItem.scaleY;

                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    let y = pointer.y - self._scrollView._y;
                    y -= self._scrollView.container.y - self._scrollView._y;
                    index = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);
                    if (index < 0 || index >= self._numButtons) {
                        // This isn't actually an error. Just dis-select.
                        index = -1;
                    }
                    if (index == self._pressedIndex && index >= 0) {
                        self._selectedIndex = index;
                        if (self.onSelected != null) {
                            self.onSelected(GetNodeByIndex(index));
                        }
                    }
                }
                
                self._pressedIndex = -1;
                //console.log("[Mouse Release] Pressed index: " + self._pressedIndex);
                UIGlobals.Active = null;
                self.UpdateColors();
                return index;
            }
        }
        const CanReparent = function(child, parent) {
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

                let y = pointer.y - self._scrollView._y;
                y -= self._scrollView.container.y - self._scrollView._y;

                let selectionIndex = Math.floor(y / UIGlobals.Sizes.TreeItemHeight);
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
        { // Drag events
            // Drag start / mouse down
            scene.input.on('dragstart', (pointer, gameObject) => {
                pointer = ConstrainPointer(pointer);

                if (gameObject != self._inputItem) { return; }
                // Drag Start:
                UIGlobals.Active = self._inputItem;

                // On Click:
                let y = pointer.y - self._scrollView._y;
                y -= self._scrollView.container.y - self._scrollView._y;
                let index = Math.floor(y / UIGlobals.Sizes.TreeItemHeight);
                if (index < 0 || index >= self._numButtons) {
                    index = -1;
                }
                self._pressedIndex = index;
                //console.log("[Drag Start] Pressed index: " + self._pressedIndex);
                self._selectedIndex = index;
                if (self.onSelected != null) {
                    self.onSelected(GetNodeByIndex(index));
                }
    
                self.UpdateColors();
            });
            scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
                pointer = ConstrainPointer(pointer);

                if (gameObject != self._inputItem) { return; }
                UIGlobals.Active = self._inputItem;

                if (!self._isDragging) { // Same as click
                    self._dragStartIndex = mouseRelease(pointer);
                    self._isDragging = true;
                    
                    if (UpdateDragIndicator(pointer)) {
                        self._orderIndicator.setActive(true).setVisible(true);
                        self._orderIndicator.setTint(UIGlobals.Colors.Dark.Blue300);
                    }
                    self.UpdateColors();

                    if (self.onSelected != null) {
                        self.onSelected(GetNodeByIndex(self._dragStartIndex));
                    }
                }
                else {
                    UpdateDragIndicator(pointer);
                }
            });
            // Drag end / mouse up
            scene.input.on('dragend', (pointer, gameObject) => {
                if (gameObject != self._inputItem) { return; }
                pointer = ConstrainPointer(pointer);

                if (UIGlobals.Active == self._inputItem) {
                    UIGlobals.Active = null;
                }
                this._pressedIndex = -1;
                //console.log("[Drag End] Pressed index: " + self._pressedIndex);


                const wasDragging = self._isDragging;
                const dragStartIndex = self._dragStartIndex;
                self._isDragging = false;
                self._dragStartIndex = -1;
                self._hoverIndex = -1; // Not sure about this

                // Mouse up:
                if (mouseRelease(pointer) < 0 && !wasDragging) {
                    self._selectedIndex = -1;
                    if (self.onSelected != null) {
                        self.onSelected(null);
                    }
                    self.UpdateColors();
                }

                if (wasDragging) {
                    let dragStopIndex = Math.floor(((pointer.y + UIGlobals.Sizes.TreeItemHeight * 0.5) - self._scrollView._y - (self._scrollView.container.y - self._scrollView._y)) / UIGlobals.Sizes.TreeItemHeight);
                    if (dragStopIndex < 0) { 
                        dragStopIndex = -1;
                    }
                    else if (dragStopIndex > self._numButtons) {
                        dragStopIndex = self._numButtons;
                    }

                    if (dragStartIndex != -1) {
                        let dragDelta = dragStopIndex - dragStartIndex;

                        if (dragDelta > 0) {
                            dragDelta -= 1;
                            dragStopIndex -= 1;
                        }

                        if (dragDelta != 0) {
                            self._selectedIndex = dragStopIndex;
                            if (self.onSelected != null) {
                                self.onSelected(GetNodeByIndex(dragStopIndex));
                            }

                            const startNode = GetNodeByIndex(dragStartIndex);
                            const stopNode = GetNodeByIndex(dragStopIndex);

                            if (CanReparent(startNode, stopNode)) {
                                startNode.SetParent(stopNode);
                            }
                            
                            //move(this._buttons, dragStartIndex, dragStopIndex);
                            self.Layout(self._x, self._y, self._width, self._height);

                            //console.log("Started dragging: " + dragStartIndex + ", stopped: " + dragStopIndex + ", delta: " + dragDelta);
                            /*if (self.onReodered != null) {
                                self.onReodered(dragStartIndex, dragStopIndex, dragDelta);
                            }*/ 
                        }
                    }
                }
            });
        }
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

        //console.log("[Update Colors] Pressed index: " + this._pressedIndex);
        //console.log("[Update Colors] Selected index: " + this._selectedIndex);
        
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

    _AddToRoots(element) {
        const length = this._roots.length;
        for (let i = 0; i < length; ++i) {
            if (this._roots[i] == element) {
                throw new Error("Duplicate root found");
            }
        }
        this._roots.push(element);
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
        this._UpdateNumButtons();
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
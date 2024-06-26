import UIGlobals from './UIGlobals.js'
import UIListBoxItem from './UIListBoxItem.js'
import UIScrollView from './UIScrollView.js'

export default class UIListBox {
    _scene = null;
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;

    _inputItem = null;
    _orderIndicator = null;

    _selectedIndex = -1;
    _hoverIndex = -1;
    _pressedIndex = -1;
    _dragStartIndex = -1;

    _scrollView = null;
    _buttons = null;
    _isDragging = false;

    onSelected = null; // onSelected(index: number, caption: string, userData: object);
    onReodered = null; // onReordered(startIndex: number, stopIndex: delta: number);
    canReorder = true;

    get _numButtons() {
        let result = 0;
        const len = this._buttons.length;
        for (let i = 0; i < len; ++i) {
            if (this._buttons[i].visible) {
                result += 1;
            }
        }
        return result;
    }

   

    constructor(scene) {
        this._scene = scene;

        const scrollView = this._scrollView = new UIScrollView(scene, this);
        this._scrollView.showHorizontal = false;
        
        this._buttons = [];

        const self = this;

        this._inputItem = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._inputItem.setDepth(UIGlobals.WidgetLayer - 2);
        this._inputItem.setOrigin(0, 0);

        this._orderIndicator = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._orderIndicator.setDepth(UIGlobals.WidgetLayer + 2);
        this._orderIndicator.setOrigin(0, 0.5);
        this._orderIndicator.setActive(false).setVisible(false);

        this._inputItem.setInteractive();
        scene.input.setDraggable(this._inputItem);

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

        const move = function(arr, old_index, new_index) {
            if (new_index >= arr.length) {
                var k = new_index - arr.length + 1;
                while (k--) {
                    arr.push(undefined);
                }
            }
            arr.splice(new_index, 0, arr.splice(old_index, 1)[0]);
            return arr; 
        };

        { // Pointer events
            this._inputItem.on("pointerover", function (pointer, localX, localY, event) {
                self._hoverIndex = -1;

                self.UpdateColors();
            });
            self._inputItem.on("pointermove", function (pointer, localX, localY, event) {
                pointer = ConstrainPointer(pointer);

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
            this._inputItem.on('wheel', (pointer, deltaX, deltaY, deltaZ, event) => {
                if (deltaY > 0) { self._scrollView.ScrollUp(); }
                else if (deltaY < 0) { self._scrollView.ScrollDown(); }
            });
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
                            self.onSelected(index, self._buttons[index].item.name, self._buttons[index].item.data);
                        }
                    }
                }
                
                self._pressedIndex = -1;
                UIGlobals.Active = null;
                self.UpdateColors();
                return index;
            }
        }
        const updateDragIndicator = function(pointer) {
            pointer = ConstrainPointer(pointer);

            let y = (pointer.y + UIGlobals.Sizes.ListBoxItemHeight * 0.5) - self._scrollView._y;
            const deltaY = self._scrollView.container.y - self._scrollView._y;
            y -= deltaY;
            const index = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);
            let itemWorldY = self._scrollView._y + deltaY + (index * UIGlobals.Sizes.ListBoxItemHeight);

            if (index >= 0 && index <= self._numButtons) {
                self._orderIndicator.setPosition(self._x, itemWorldY);
                self._orderIndicator.setScale(self._scrollView._maskRect.width, UIGlobals.Sizes.ListBoxOrderIndicator);
                return true;
            }

            return false;
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
                let index = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);
                if (index < 0 || index >= self._numButtons) {
                    index = -1;
                }
                self._pressedIndex = index;
    
                self.UpdateColors();
            });
            scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
                pointer = ConstrainPointer(pointer);

                if (gameObject != self._inputItem) { return; }
                UIGlobals.Active = self._inputItem;

                if (!self._isDragging) { // Same as click
                    self._dragStartIndex = mouseRelease(pointer);
                    self._isDragging = true;

                    if (updateDragIndicator(pointer)) {
                        self._orderIndicator.setActive(self.canReorder).setVisible(self.canReorder);
                        self._orderIndicator.setTint(UIGlobals.Colors.Dark.Blue300);
                    }
                    self.UpdateColors();
                }
                else {
                    updateDragIndicator(pointer);
                }

            });
            // Drag end / mouse up
            scene.input.on('dragend', (pointer, gameObject) => {
                if (gameObject != self._inputItem) { return; }
                pointer = ConstrainPointer(pointer);

                // Drag end:
                const wasDragging = self._isDragging;
                self._isDragging = false;
                self._hoverIndex = -1;

                // Mouse up:
                if (mouseRelease(pointer) < 0 && !wasDragging) {
                    self._selectedIndex = -1;
                    if (self.onSelected != null) {
                        self.onSelected(-1, "", null);
                    }
                    self.UpdateColors();
                }
                const dragStartIndex = self._dragStartIndex;

                if (wasDragging) {
                    let dragStopIndex = Math.floor(((pointer.y + UIGlobals.Sizes.ListBoxItemHeight * 0.5) - self._scrollView._y - (self._scrollView.container.y - self._scrollView._y)) / UIGlobals.Sizes.ListBoxItemHeight);
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

                        if (dragDelta != 0 && self.canReorder) {
                            self._selectedIndex = dragStopIndex;
                            if (self.onSelected != null) {
                                self.onSelected(dragStopIndex, self._buttons[dragStopIndex].item.name, self._buttons[dragStopIndex].item.data);
                            }

                            move(this._buttons, dragStartIndex, dragStopIndex);
                            self.Layout(self._x, self._y, self._width, self._height);

                            //console.log("Started dragging: " + dragStartIndex + ", stopped: " + dragStopIndex + ", delta: " + dragDelta);
                            if (self.onReodered != null) {
                                self.onReodered(dragStartIndex, dragStopIndex, dragDelta);
                            } 
                        }
                    }
                }
            });
        }
        
    }

    UpdateColors() {
        this._scrollView.UpdateColors();

        const tnitColors = [
            UIGlobals.Colors.BackgroundLayer1,
            UIGlobals.Colors.BackgroundLayer1AndAHalf
        ];
        const highlightColors = [
           UIGlobals.Colors.Dark.Blue300,
           UIGlobals.Colors.Dark.Blue200,
        ];
        const textTint = UIGlobals.Colors.Text;
        const selectionIndex = this._selectedIndex;
        const hoverIndex = this._hoverIndex;
        for (let i = 0; i < this._buttons.length; ++i) {
            this._buttons[i].SetTint(tnitColors[i % 2], textTint);
            if (i == selectionIndex) {
                this._buttons[i].SetTint(UIGlobals.Colors.Dark.Blue100, textTint);
            }
            else if (i == hoverIndex && UIGlobals.Active == null) {
                if (!this._isDragging) {
                    this._buttons[i].SetTint(highlightColors[i % 2], textTint);
                }
            }
        }

        this._orderIndicator.setVisible(this._isDragging && this.canReorder);
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

        const buttonHeight = UIGlobals.Sizes.ListBoxItemHeight;
        const scrollSize = UIGlobals.Sizes.ScrollTrackSize;
        if (this._scrollView.showHorizontal) {
            width -= scrollSize;
        }
        else if (this._scrollView.showVertical) {
            height -= scrollSize;
        }

        x = 0;
        y = 0;
        for (let i = 0; i < this._buttons.length; ++i) {
            if (!this._buttons[i].visible) {
                break;
            }
            this._buttons[i].Layout(x, y + (buttonHeight * i), width);
        }

        this._scrollView.Layout(this._x, this._y, this._width, this._height);


        this.UpdateColors();
    }

    SetVisibility(value) {
        this._scrollView.SetVisibility(value);
        this._inputItem.setActive(value).setVisible(value);
        if (!value) {
            this._orderIndicator.setActive(value).setVisible(this._isDragging);
        }
    }

    Clear() {
        for (let i = 0; i < this._buttons.length; ++i) {
            this._buttons[i].Hide();
        }
    }

    Add(itemName, itemUserData = null) {
        const item = {
            name: itemName,
            data: itemUserData
        };

        let button = null;
        for (let i = 0; i < this._buttons.length; ++i) {
            if (!this._buttons[i].visible) {
                button = this._buttons[i];
                button.Show();
                break;
            }
        }
        if (button == null) {
            button = new UIListBoxItem(this._scene);
            this._buttons.push(button);
            button.AddToContainer(this._scrollView.container);
        }
        button.item = item;
        button.UpdateText(itemName);

        // Call Layout manually elsewhewre
        //this.Layout(this._x, this._y, this._width, this._height);
    } 
}
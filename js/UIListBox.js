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

    _scrollView = null;
    _buttons = null;
    _isDragging = false;

    onSelected = null; // onSelected(index: number, caption: string, userData: object);
    onReodered = null; // onReordered(delta: number, index: number, caption: string, userData: object);

    constructor(scene) {
        this._scene = scene;

        this._scrollView = new UIScrollView(scene, this);
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

        { // Pointer events
            this._inputItem.on("pointerover", function (pointer, localX, localY, event) {
                if (UIGlobals.Active == null) {
                    UIGlobals.Hot = self._inputItem;
                }
                self._hoverIndex = -1;

                self.UpdateColors();
            });
            self._inputItem.on("pointermove", function (pointer, localX, localY, event) {
                UIGlobals.Active = self._inputItem;
                const count = self._buttons.length;

                let y = pointer.y - self._scrollView._y;
                y -= self._scrollView.container.y - self._scrollView._y;

                const selectionIndex = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);
                if (selectionIndex < 0 || selectionIndex >= count) {
                    throw new Error("Listbox invalid selection");
                }
                self._hoverIndex = selectionIndex;

                self.UpdateColors();
            });
            this._inputItem.on("pointerout", function (pointer, event) {
                if (UIGlobals.Hot == self._inputItem) {
                    UIGlobals.Hot = null;
                }
                self._hoverIndex = -1;

                self.UpdateColors();
            });
        }
        { // Drag events
            // Drag start / mouse down
            scene.input.on('dragstart', (pointer, gameObject) => {
                if (gameObject != self._inputItem) { return; }
                // Drag Start:
                UIGlobals.Active = self._inputItem;
                // self._isDragging = true;
                //self._hoverIndex = -1;

                // On Click:
                let y = pointer.y - self._scrollView._y;
                y -= self._scrollView.container.y - self._scrollView._y;
                const index = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);
                if (index < 0 || index >= self._buttons.length) {
                    // This isn't actually an error. Just dis-select.
                    throw new Error("Listbox invalid selection");
                }
                self._pressedIndex = index;
    
                self.UpdateColors();
            });
            const mouseRelease = function(pointer) {
                if (UIGlobals.Active != null && UIGlobals.Active == self._inputItem) {
                    let left = self._inputItem.x;
                    let right = left + self._inputItem.scaleX;
                    let top = self._inputItem.y;
                    let bottom = top + self._inputItem.scaleY;
    
                    if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                        let y = pointer.y - self._scrollView._y;
                        y -= self._scrollView.container.y - self._scrollView._y;
                        const index = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);
                        if (index < 0 || index >= self._buttons.length) {
                            // This isn't actually an error. Just dis-select.
                            throw new Error("Listbox invalid selection");
                        }
                        if (index == self._pressedIndex) {
                            if (self.onSelected != null) {
                                self.onSelected(index, )
                            }
                            self._selectedIndex = index;
                        }
                    }
                    
                    self._pressedIndex = -1;
                    UIGlobals.Active = null;
                    self.UpdateColors();
                }
            }
            const updateDragIndicator = function(pointer) {
                let y = (pointer.y + UIGlobals.Sizes.ListBoxItemHeight * 0.5) - self._scrollView._y;
                y -= self._scrollView.container.y - self._scrollView._y;
                const index = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);

                let itemWorldY = self._scrollView._y + (index * UIGlobals.Sizes.ListBoxItemHeight);

                if (index >= 0 && index < self._buttons.length) {
                    self._orderIndicator.setPosition(self._x, itemWorldY);
                    self._orderIndicator.setScale(self._width, UIGlobals.Sizes.ListBoxOrderIndicator);
                    self._orderIndicator.setActive(true).setVisible(true);
                    return true;
                }

                return false;
            }
            scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
                if (gameObject != self._inputItem) { return; }
                UIGlobals.Active = self._inputItem;

                let y = pointer.y - self._scrollView._y;
                y -= self._scrollView.container.y - self._scrollView._y;
                const index = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);

                if (!self._isDragging) { // Same as click
                    mouseRelease(pointer);
                    self._isDragging = true;

                    if (updateDragIndicator(pointer)) {
                        self._orderIndicator.setActive(true).setVisible(true);
                    }
                    self.UpdateColors();
                }
                else {
                    updateDragIndicator(pointer);
                }

                console.log("dragging: " + index);
            });
            // Drag end / mouse up
            scene.input.on('dragend', (pointer, gameObject) => {
                if (gameObject != self._inputItem) { return; }

                // Drag end:
                self._isDragging = false;
                self._hoverIndex = -1;

                // Mouse up:
                mouseRelease(pointer);
            });
        }
        
    }

    UpdateColors() {
        this._scrollView.UpdateColors();

        const tnitColors = [
            UIGlobals.Colors.BackgroundLayer1,
            UIGlobals.Colors.BackgroundLayer1AndAHalf
        ];
        const selectionIndex = this._selectedIndex;
        const hoverIndex = this._hoverIndex;
        for (let i = 0; i < this._buttons.length; ++i) {
            if (i == selectionIndex) {
                this._buttons[i].SetTint(0xff0000);
            }
            else if (i == hoverIndex) {
                this._buttons[i].SetTint(0x00ff00);
            }
            else {
                this._buttons[i].SetTint(tnitColors[i % 2]);
            }
        }

        this._orderIndicator.setVisible(this._isDragging);
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
        const buttonHeight = UIGlobals.Sizes.ListBoxItemHeight;

        x = 0;
        y = 0;
        width = this._scrollView._width;
        for (let i = 0; i < this._buttons.length; ++i) {
            if (!this._buttons[i].visible) {
                break;
            }
            this._buttons[i].Layout(x, y + (buttonHeight * i), width);
        }

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

        button.UpdateText(itemName);

        this.Layout(this._x, this._y, this._width, this._height);
    } 
}
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

    _selectedIndex = -1;

    _scrollView = null;
    _buttons = null;
    _itemList = null; // doubly linked list, not array!

    onSelected = null; // onSelected(index: number, caption: string, userData: object);
    onReodered = null; // onReordered(delta: number, index: number, caption: string, userData: object);

    constructor(scene) {
        this._scene = scene;

        this._scrollView = new UIScrollView(scene, this);
        this._scrollView.showHorizontal = false;
        
        this._buttons = [];
        this._itemList = null;

        const self = this;

        this._inputItem = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._inputItem.setDepth(UIGlobals.WidgetLayer - 2);
        this._inputItem.setOrigin(0, 0);

        this._inputItem.setInteractive();
        scene.input.setDraggable(this._inputItem);



        /*this._inputItem.on("pointerover", function (pointer, localX, localY, event) {
            if (UIGlobals.Active == null) {
                UIGlobals.Hot = self._inputItem;
            }


            self.UpdateColors();
        });
        this._inputItem.on("pointerout", function (pointer, event) {
            if (UIGlobals.Hot == self._inputItem) {
                UIGlobals.Hot = null;
            }


            self.UpdateColors();
        });*/

        self._inputItem.on("pointerdown", function (pointer, localX, localY, event) {
            UIGlobals.Active = self._inputItem;
            const count = self._buttons.length;

            let y = pointer.y - self._scrollView._y;
            y -= self._scrollView.container.y - self._scrollView._y;

            const selectionIndex = Math.floor(y / UIGlobals.Sizes.ListBoxItemHeight);
            if (selectionIndex < 0 || selectionIndex >= count) {
                throw new Error("Listbox invalid selection");
            }

            self._selectedIndex = selectionIndex;

            self.UpdateColors();
        });

        scene.input.on('dragstart', (pointer, gameObject) => {
            if (gameObject != self._inputItem) { return; }
            UIGlobals.Active = self._inputItem;
        });

        scene.input.on('dragend', (pointer, gameObject) => {
            if (gameObject != self._inputItem) { return; }

            if (UIGlobals.Active == self._inputItem) {
                UIGlobals.Active = null;
            }

        });

        scene.input.on('drag', (pointer, gameObject, dragX, dragY) => {
            if (gameObject != self._inputItem) { return; }

        });
    }

    UpdateColors() {
        this._scrollView.UpdateColors();

        const tnitColors = [
            UIGlobals.Colors.BackgroundLayer1,
            UIGlobals.Colors.BackgroundLayer1AndAHalf
        ];
        const selectionIndex = this._selectedIndex;
        for (let i = 0; i < this._buttons.length; ++i) {
            if (i == selectionIndex) {
                this._buttons[i].SetTint(0xff0000);
            }
            else {
                this._buttons[i].SetTint(tnitColors[i % 2]);
            }
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
    }

    SetVisibility(value) {
        this._scrollView.SetVisibility(value);
    }

    Clear() {
        this._itemList = null;
        for (let i = 0; i < this._buttons.length; ++i) {
            this._buttons[i].Hide();
        }
    }

    Add(itemName, itemUserData = null) {
        const item = {
            prev: null,
            next: null,
            name: itemName,
            data: itemUserData
        };

        if (this._itemList == null) {
            this._itemList = item;
        }
        else {
            let iter = this._itemList;
            while (iter != null) {
                if (iter.next == null) {
                    iter.next = item;
                    item.prev = iter;
                    break;
                }
                iter = iter.next;
            }
        }

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
import UIImageButton from './UIImageButton.js'
import UIGlobals from './UIGlobals.js'
import UIListBox from './UIListBox.js'
import Clip from './Animation.js'
import UIView from './UIView.js'

export default class AnimationsView extends UIView {
    _list = null;

    _header = null;
    _buttons = [];

    selectedAnimation = null;
    _selectedIndex = -1;
    _keyframesView = null;

    onSelectionChanged = null; // (selectedAnimation)

    get active() {
        return this.selectedAnimation;
    }

    set active(valeu) {
        if (this.selectedAnimation != valeu) {
            if (this.onSelectionChanged != null) {
                this.onSelectionChanged(valeu);
            }
            if (this._keyframesView != null) {
                this._keyframesView.FocusOn(valeu);
            }
        }
        this.selectedAnimation = valeu;
        this._selectedIndex = this._list.FindUserDataIndex( valeu);
    }

    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._list = new UIListBox(scene);
        this._list.canReorder = false;

        this._list.onSelected = (index, caption, userData) => {
            self.active = userData;
        };

        this._header =  scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._header.setDepth(UIGlobals.WidgetLayer);
        this._header.setOrigin(0, 0);

        const newAnimButton = new UIImageButton(scene, "SmallIconHierarchyNew.png", () => {
            const anim = new Clip();
            self._list.Add(anim.name, anim);
            self.Layout();
            self.UpdateColors();
        });
        const deleteAnimButton = new UIImageButton(scene, "SmallIconTrash.png", () => {
            if (self.selectedAnimation !== null) {
                const removeIndex = self._list.FindUserDataIndex(self.selectedAnimation);

                self.Deselect();
                if (removeIndex >= 0) {
                    self._list._buttons[removeIndex].Destroy();
                }
                self.Layout();
                self.UpdateColors();
            }
        });
        const deselectButton = new UIImageButton(scene, "SmallIconDeselect.png", () => {
            self.Deselect();
        });
        this._buttons.push(deselectButton);
        this._buttons.push(deleteAnimButton);
        this._buttons.push(newAnimButton);
    }

    Deselect() {
        this._list.Deselect();
        this.active = null;
    }

    UpdateColors() {
        this._list.UpdateColors();
        this._header.setTint(UIGlobals.Colors.BackgroundLayer0);
        const length = this._buttons.length;
        for (let i = 0; i < length; ++i) {
            this._buttons[i].UpdateColors();
        }
    }

    Layout(x, y, width, height) {
        if (x === undefined) {  x = this._x; }
        if (y === undefined) {  y = this._y; }
        if (width === undefined) {  width = this._width; }
        if (height === undefined) {  height = this._height; }
        
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        const headerHeight = UIGlobals.Sizes.TreeFooterHeight;
        const size = headerHeight - UIGlobals.Sizes.TreeFooterMargin * 2;
        const padding = UIGlobals.Sizes.TreeFooterPadding;
        const margin = UIGlobals.Sizes.TreeFooterMargin;

        this._list.Layout(x, y + headerHeight, width, height - headerHeight);

        this._header.setPosition(x, y);
        this._header.setScale(width, headerHeight);

        let xPos = x + width - padding - (size / 2);// x + padding + (size / 2);
        let yPos = y + margin + (size / 2);

        const length = this._buttons.length;
        for (let i = 0; i < length; ++i) {
            this._buttons[i].Layout(xPos, yPos, size, size);
            xPos -= (size + padding);
        }
    }

    SetVisibility(value) {
        this._list.SetVisibility(value);

        this._header.setActive(value).setVisible(value);

        const length = this._buttons.length;
        for (let i = 0; i < length; ++i) {
            this._buttons[i].SetVisibility(value);
        }
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }

    UpdateNames() {
        const listButtons = this._list._buttons;
        const length = listButtons.length;
        for (let i = 0; i < length; ++i) {
            listButtons[i]._labelText.text = listButtons[i].item.data.name;
        }
    }
}
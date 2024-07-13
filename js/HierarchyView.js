import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITree from './UITree.js'
import UIImageButton from './UIImageButton.js'
import XForm from './Transform.js'
import SpriteImg from './Sprite.js'

export default class HierarchyView extends UIView {
    static _nameCounter = 0;
    _tree = null;
    _footer = null;
    _buttons = [];
    _active = null;

    _drawOrderView = null;
    _drawOrderToHiararchyNodeMap = null;
    _sceneView = null;


    onSelectionChanged = null; // (oldActive, newActive)

    constructor(scene, parent, drawOrderView, sceneView) {
        super(scene, parent);
        const self = this;
        this._sceneView = sceneView;

        drawOrderView._hierarchyView = this;
        self._drawOrderView = drawOrderView;
        this._drawOrderToHiararchyNodeMap = new Map();

        this._tree = new UITree(scene);
        this._tree.canReorder = false;

        this._tree.onRearranged = (targetNode) => {
            /*self._tree.ForEach((node) => {
                node._userData.transform.ApplyTransform(node._userData.sprite.sprite);
            })*/
           self._UpdateTransforms();
           self._sceneView.UpdateColors();
        }

        this._tree.onSelected = (treeNode) => {
            self.active = treeNode;
            self._sceneView.UpdateColors();
        }

        this._footer =  scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._footer.setDepth(UIGlobals.WidgetLayer);
        this._footer.setOrigin(0, 0);

        const newNodeButton = new UIImageButton(scene, "SmallIconHierarchyNew.png", () => {
            const hierarchyNode = self.AddNewNode();
            self._sceneView.UpdateColors();
        });
        const deleteNodeButton = new UIImageButton(scene, "SmallIconTrash.png", () => {
            self.Delete();
            self._sceneView.UpdateColors();
        });
        const deselectButton = new UIImageButton(scene, "SmallIconDeselect.png", () => {
            self.Deselect();
            self._sceneView.UpdateColors();
        });
        this._buttons.push(deselectButton);
        this._buttons.push(deleteNodeButton);
        this._buttons.push(newNodeButton);
    }

    _UpdateTransforms() {
        const ui = {
            x: UIGlobals.Sizes.ToolboxWidth, y: UIGlobals.Sizes.EditorBarHeight,
            rotation: 0,
            scaleX: 1, scaleY: 1
        };
        const camera = this._sceneView._cameraTransform;
        const view = XForm.Mul(ui, camera, null);

        this._tree.ForEach((node) => {
            node._userData.transform.ApplyTransform(node._userData.sprite.sprite, view);
        });

        this._sceneView.UpdateActiveShelf();
    }

    get active() {
        return this._active;
    }

    set active(valeu) {
        if (this._active != valeu) {
            if (this.onSelectionChanged != null) {
                this.onSelectionChanged(this._active, valeu);
            }
        }
        this._active = valeu;
    }

    UpdateSortingIndex(node, newDepth) {
        this._drawOrderToHiararchyNodeMap.get(node)._userData.sprite.sprite.setDepth(newDepth);
    }

    Delete(toRemove) {
        const self = this;
        if (toRemove === undefined) {
            if (this.active != null) {
                this.active.Recursive((node) => {
                    self.Delete(node);
                });
            }
            return;
        }

        //if (toRemove._userData !== null) {
            this._drawOrderToHiararchyNodeMap.delete(toRemove._userData.drawOrder);
            toRemove._userData.drawOrder.Destroy();
            toRemove._userData.sprite.Destroy();
        //}
        toRemove._userData = null;

        this.Deselect();
        this._tree.Remove(toRemove);
        this._UpdateTransforms();
        
        this.active = null;
    }

    Deselect() {
        this._tree.Deselect();
        this.active = null;
    }

    AddNewNode(nodeName) {
        if (!nodeName) {
            nodeName = "Unnamed Node " + (HierarchyView._nameCounter++);;
        }

        let parent = null;
        if (this.active) {
            parent = this.active;
        }
        
        const newNode = this._tree.Add(nodeName);
        if (parent) {
            newNode.SetParent(parent);
        }

        const transformNode = new XForm(newNode);
        const spriteNode = new SpriteImg(newNode, this._drawOrderView);
        newNode._userData.drawOrder = this._drawOrderView.Add(newNode.name);
        this._drawOrderToHiararchyNodeMap.set(newNode._userData.drawOrder, newNode);

        this.Layout(this._x, this._y, this._width, this._height);
        this._UpdateTransforms();
        return newNode;
    }

    UpdateColors() {
        this._tree.UpdateColors();
        this._footer.setTint(UIGlobals.Colors.BackgroundLayer0);
        const length = this._buttons.length;
        for (let i = 0; i < length; ++i) {
            this._buttons[i].UpdateColors();
        }
    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        const footerHeight = UIGlobals.Sizes.TreeFooterHeight;
        const size = footerHeight - UIGlobals.Sizes.TreeFooterMargin * 2;
        const padding = UIGlobals.Sizes.TreeFooterPadding;
        const margin = UIGlobals.Sizes.TreeFooterMargin;

        this._tree.Layout(x, y + footerHeight, width, height - footerHeight);

        this._footer.setPosition(x, y);
        this._footer.setScale(width, footerHeight);

        let xPos = x + width - padding - (size / 2);// x + padding + (size / 2);
        let yPos = y + margin + (size / 2);

        const length = this._buttons.length;
        for (let i = 0; i < length; ++i) {
            this._buttons[i].Layout(xPos, yPos, size, size);
            xPos -= (size + padding);
        }
    }

    SetVisibility(value) {
        this._tree.SetVisibility(value);
        this._footer.setActive(value).setVisible(value);

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
}
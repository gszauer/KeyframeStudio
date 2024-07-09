import XForm from './Transform.js'
import ColorRGB from './ColorRGB.js'
import UIGlobals from './UIGlobals.js'
import DrawOrderView from './DrawOrderView.js'

export default class SpriteImg {
    _uiTreeNode = null; // Node that this sprite is attached to

    x = 0;
    y = 0;
    width = 0;
    height = 0;
    pivotX = 0;
    pivotY = 0;
    alpha = 1;
    visible = true;
    color = null;
    sprite = null;

    constructor(treeNode, drawOrderView = null) {
        if (!treeNode) {
            throw new Error("Sprite must be attached to tree node");
        }
        this._uiTreeNode = treeNode;

        const scene = treeNode._tree._scene;

        this.color = new ColorRGB(1, 1, 1);
        this.sprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this.sprite.setDepth(UIGlobals.WidgetLayer - drawOrderView.count);
        this.sprite.setOrigin(0, 0);

        if (!treeNode._userData) {
            treeNode._userData = {
                sprite: this
            };
        }
        else {
            treeNode._userData.sprite = this;
        }
    }

    get transform() {
        if (!this._uiTreeNode._userData.transform) { // If no transform is attached, add one!
            return this._uiTreeNode._userData.transform = new XForm(this._uiTreeNode);
        }
        return this._uiTreeNode._userData.transform;
    }

    Destroy() { // Hooked up in hierarchyview delete
        this.sprite.destroy();
        this.sprite = null;
    }

}
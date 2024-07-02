import XForm from './Transform.js'
import ColorRGB from './ColorRGB.js'
import UIGlobals from './UIGlobals.js'

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

    static Head = null;
    _prev = null;
    _next = null;

    constructor(treeNode = null) {
        this.color = new ColorRGB(1, 1, 1);
        this.sprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this.sprite.setDepth(UIGlobals.ContentLayer);
        this.sprite.setOrigin(0, 0);

        if (!treeNode) {
            throw new Error("Sprite must be attached to tree node");
        }
        this._uiTreeNode = treeNode;

        if (!treeNode._userData) {
            treeNode._userData = {
                sprite: this
            };
        }
        else {
            treeNode._userData.sprite = this;
        }

        if (SpriteImg.Head == null) {
            SpriteImg.Head = this;
        }
        else {
            let tail = SpriteImg.Head;
            while (tail != null && tail._next != null) {
                tail = tail._next;
            }
            tail._next = this;
            this._prev = tail;
        }
    }

    get transform() {
        if (!this._uiTreeNode._userData.transform) { // If no transform is attached, add one!
            return this._uiTreeNode._userData.transform = new XForm(this._uiTreeNode);
        }
        return this._uiTreeNode._userData.transform;
    }

    Destroy() { // TODO: This needs to be hooked up to UITreeNode, maybe with a callback?!?
        if (this._prev != null) {
            this._prev._next = this._next;
        }
        if (this._next != null) {
            this._next._prev = this._prev;
        }
        if (this == SpriteImg.Head) {
            SpriteImg.Head = this._next;
        }

        this.sprite.destroy();
        this.sprite = null;
    }

}
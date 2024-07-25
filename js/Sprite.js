import XForm from './Transform.js'
import ColorRGB from './ColorRGB.js'
import UIGlobals from './UIGlobals.js'

export default class SpriteImg {
    _uiTreeNode = null; // Node that this sprite is attached to

    x = 0;
    y = 0;
    width = 1;
    height = 1;
    pivotX = 0.5;
    pivotY = 0.5;
    alpha = 1;
    visible = true;
    color = null;
    sprite = null;

    enabled = true;

    uid = "Frame 0";
    static uidGen = 0;

    constructor(treeNode, sceneView, drawOrderView = null) {
        if (!treeNode) {
            throw new Error("Sprite must be attached to tree node");
        }
        this._uiTreeNode = treeNode;
        this.uid = "Frame " + (++SpriteImg.uidGen);

        const scene = treeNode._tree._scene;
        const assetsView = sceneView._assetsView;

        let textureName = UIGlobals.Atlas;
        let textureFrame = UIGlobals.Solid;

        if (assetsView.atlasTextureName != null) {
            textureName = assetsView.atlasTextureName;
            textureFrame = this.uid;

            const _textures = scene.textures;
            const _texture = _textures.get(textureName);
            if (_texture.has(textureFrame)) {
                _texture.remove(textureFrame);
            }

            this.width = _texture.source[0].width;
            this.height = _texture.source[0].height;

            _texture.add(textureFrame, 0, this.x, this.y, this.width, this.height);
        }
        this.sprite = scene.add.sprite(0, 0, textureName, textureFrame);

        this.color = new ColorRGB(1, 1, 1);
        let layerCount = 0;
        if (drawOrderView !== null) {
            layerCount = drawOrderView.count;
        }
        this.sprite.setDepth(UIGlobals.WidgetLayer - layerCount);
        this.sprite.setOrigin(0, 0);
        this.sprite.setMask(sceneView.mask);

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

    get isVisible() {
        return this.enabled && this.visible;
    }

    Disable() {
        this.enabled = false;
        this.sprite.setActive(this.isVisible).setVisible(this.isVisible);
    }

    Enable() {
        this.enabled = true;
        this.sprite.setActive(this.isVisible).setVisible(this.isVisible);
    }

    SetVisibility(value) {
        this.visible = value;
        this.sprite.setActive(this.isVisible).setVisible(this.isVisible);
    }
}
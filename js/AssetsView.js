import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITextButton from './UITextButton.js'
import UIStringBox from './UIStringBox.js'
import UIPopup from './UIPopup.js'

export default class AssetsView extends UIView {
    _hierarchyView = null;
    _inspectorView = null;
    _backgroundSprite = null;

    _spriteSheetLabel = null;
    _spriteSheetTextField = null;
    _spriteSheetButton = null;

    _altasLabel = null;
    _atlasTextField = null;
    _atlasButton = null;

    _previewLabel = null;
    _previewBorder = null;
    _previewBackground = null;
    _previewImage = null;

    frameMap = new Map();
    fileInput = null;
    jsonInput = null;

    onImageChanged = null; // (imgName, imgWidth, imgHeight, imgObject);
    _visible = false;
    _scene = null;

    static AtlasIndex = 0;

    _dummyImage = null;
    atlasTextureName = null;
    get atlasTexture() {
        if (this._previewImage !== null) {
            return this._previewImage;
        }
        if (this._dummyImage == null) {
            this._dummyImage = this._scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
            this._dummyImage.setActive(false).setVisible(false);
        }

        return this._dummyImage;
    }

    UpdateFrames() {
        if (this.atlasTextureName === null) {
            return;
        }
        const atlasTextureName = this.atlasTextureName;
        const textures = this._scene.textures;
        const texture = textures.get(atlasTextureName);

        this._hierarchyView.ForEach((node, depth) => {
            const sprite = node._userData.sprite;
            const frameName = sprite.uid;
            
            if (texture.has(frameName)) {
                texture.remove(frameName);
            }
            texture.add(frameName, 0, sprite.x, sprite.y, sprite.width, sprite.height);
            //const frame = textures.getFrame(atlasTextureName, frameName);
        });

        this._hierarchyView.ForEach((node, depth) => {
            const sprite = node._userData.sprite;
            sprite.sprite.setFrame(sprite.uid);
            sprite.sprite.setOrigin(sprite.pivotX, sprite.pivotY);

            sprite.sprite.setTint(sprite.color.color);
            //sprite.sprite.setAlpha(sprite.alpha, sprite.alpha, sprite.alpha, sprite.alpha);
        });
    }


    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;
        this._scene = scene;

        this.fileInput = document.getElementById("file-upload");
        this.jsonInput = document.getElementById("json-upload");

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0, 0);

        this._spriteSheetLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, "Sprite Sheet (png)");
        this._spriteSheetLabel.setDepth(UIGlobals.WidgetLayer);

        this._spriteSheetTextField = new UIStringBox(scene, "");

        this._browseSpriteSheet = new UITextButton(scene, "Browse", (btn) => {
            self.fileInput.click();
        });

        this._altasLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, "Sprite Atlas (json)");
        this._altasLabel.setDepth(UIGlobals.WidgetLayer);

        this._atlasTextField = new UIStringBox(scene, "");

        this._atlasButton = new UITextButton(scene, "Browse", (btn) => {
            self.jsonInput.click();
        });

        this._previewLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, "Sprite Sheet Preview");
        this._previewLabel.setDepth(UIGlobals.WidgetLayer);

        this._previewBorder = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._previewBorder.setDepth(UIGlobals.WidgetLayer);
        this._previewBorder.setOrigin(0, 0);

        this._previewBackground = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._previewBackground.setDepth(UIGlobals.WidgetLayer);
        this._previewBackground.setOrigin(0, 0);

        this.jsonInput.onchange = (event) => {
            event.preventDefault();

            try {
                const files = self.jsonInput.files;
                if (!files.length) {
                    console.log('No file selected!');
                    return;
                }
                let file = files[0];
                let reader = new FileReader();

                reader.onload = (event) => {
                    const obj = JSON.parse(event.target.result);

                    self.frameMap = new Map();

                    if (obj.hasOwnProperty("textures")) {
                        const textures = obj.textures;
                        if (textures.length > 0) {
                            const texture = textures[0];
                            if (texture.hasOwnProperty("frames")) {
                                const frames = texture.frames;
                                const framesLen = frames.length;
                                for (let i = 0; i < framesLen; ++i) {
                                    const frame = frames[i];

                                    let frameName = "";
                                    const frameData = { x: 0, y: 0, width: 0, height: 0, px: 0.5, py: 0.5 };

                                    if (frame.hasOwnProperty("filename")) {
                                        frameName = frame.filename;
                                    }
                                    if (frame.hasOwnProperty("frame")) {
                                        const frm = frame.frame;
                                        if (frm.hasOwnProperty("x")) {
                                            frameData.x = frm.x;
                                        }
                                        if (frm.hasOwnProperty("y")) {
                                            frameData.y = frm.y;
                                        }
                                        if (frm.hasOwnProperty("w")) {
                                            frameData.width = frm.w;
                                        }
                                        if (frm.hasOwnProperty("h")) {
                                            frameData.height = frm.h;
                                        }
                                    }
                                    if (frame.hasOwnProperty("pivot")) {
                                        const pvt = frame.pivot;
                                        if (pvt.hasOwnProperty("x")) {
                                            frameData.px = pvt.x;
                                        }
                                        if (pvt.hasOwnProperty("y")) {
                                            frameData.py = pvt.y;
                                        }
                                    }

                                    self.frameMap.set(frameName, frameData);
                                }
                            }
                        }
                    }

                    self._atlasTextField.text = file.name;

                    const popup = new UIPopup(scene);
                    popup._forceWidth = 250;
                    for (let [key, value] of  self.frameMap.entries()) {
                        popup.Add(key, null);
                    }

                    const inspectorView = self._inspectorView;
                    popup.onSelect = (label, _popup) => {
                        if (inspectorView._focused === null) {
                            return;
                        }

                        const data = self.frameMap.get(label);
                        const sprt = inspectorView._focused._userData.sprite;
                        sprt.x = data.x;
                        sprt.y = data.y;
                        sprt.width = data.width;
                        sprt.height = data.height;
                        sprt.pivotX = data.px;
                        sprt.pivotY = data.py;

                        self.UpdateFrames();
                        inspectorView.FocusOn(inspectorView._focused);
                    };

                    self._inspectorView.UpdatePresetPopup(popup);
                };
                reader.readAsText(file);
            } catch (err) {
                console.error(err);
            }
            this._hierarchyView.Deselect();
        };

        this.fileInput.onchange = (event) => {
            event.preventDefault();
            
            // Check if any files are selected
            const selectedFiles = self.fileInput.files;
            for (let i = 0; i < selectedFiles.length; i++) {
                const reader = new FileReader();
                const fileName = selectedFiles[i].name;
        
                reader.onload = () => {
                    const base64 = reader.result;
                    const name = this.atlasTextureName = 'Atlas' + (++AssetsView.AtlasIndex);

                    scene.textures.addBase64(name, base64).once(Phaser.Textures.Events.LOAD, () => {
                        if (self._previewImage !== null) {
                            self._previewImage.destroy();
                        }
                        self._previewImage = scene.add.sprite(0, 0, name);
                        self._previewImage.setDepth(UIGlobals.WidgetLayer);
                        self._previewImage.setOrigin(0, 0);
                        self._previewImage.setActive(this._visible).setVisible(this._visible);
                        self.Layout();

                        self._spriteSheetTextField.text = fileName;

                        self._hierarchyView.ForEach((node, depth) => {
                            const sprite = node._userData.sprite.sprite;
                            sprite.setTexture(name);
                        });

                        self.UpdateFrames();
                    });
                };
        
                reader.readAsDataURL(selectedFiles[i]);

                return; // Only process 1 file
            }
            this._hierarchyView.Deselect();
        }
    }

    UpdateColors() {
        this._backgroundSprite.setTint(UIGlobals.Colors.BackgroundLayer1);
        this._browseSpriteSheet.UpdateColors();
        this._atlasButton.UpdateColors();
        this._spriteSheetTextField.UpdateColors();
        this._atlasTextField.UpdateColors();

        const borderTint = UIGlobals.Colors.ElementBorderTintIdle;
        const bgTint = UIGlobals.Colors.BackgroundLayer0;

        this._spriteSheetLabel.setTint(borderTint);
        this._altasLabel.setTint(borderTint);
        this._previewLabel.setTint(borderTint);

        this._previewBorder.setTint(borderTint);
        this._previewBackground.setTint(bgTint);
    }

    Layout(x, y, width, height) {
        if (x === undefined) {  x = this._x; }
        if (y === undefined) {  y = this._y; }
        if (width === undefined) {  width = this._width; }
        if (height === undefined) {  height = this._height; }

        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        super.Layout(x, y, width, height);

        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        this._backgroundSprite.setPosition(x, y);
        this._backgroundSprite.setScale(width, height);

        const margin = UIGlobals.Sizes.InspectorTitleMargin;
        const skip = UIGlobals.Sizes.InspectorTitleSkip;

        x += margin;
        y += margin;

        width = width - margin * 2;

        const xReset = x;
        const wReset = width;

        const browseButtonWidth = 100;
        {
            width -= browseButtonWidth - 4;

            this._spriteSheetLabel.setPosition(x, y);
            y = y + this._spriteSheetLabel.height + skip;

            this._spriteSheetTextField.Layout(x, y, width);

            x += width + 4;
            width = browseButtonWidth - margin;

            this._browseSpriteSheet.Layout(x, y, width, this._spriteSheetTextField._height);
            y += this._spriteSheetTextField._height + skip;
        }

        x = xReset;
        width = wReset;
        {
            width -= browseButtonWidth - 4;

            this._altasLabel.setPosition(x, y);
            y = y + this._altasLabel.height + skip;

            this._atlasTextField.Layout(x, y, width);

            x += width + 4;
            width = browseButtonWidth - margin;

            this._atlasButton.Layout(x, y, width, this._atlasTextField._height);
            y += this._atlasTextField._height + skip;
        }

        x = xReset;
        width = wReset;
        {
            this._previewLabel.setPosition(x, y);
            y = y + this._previewLabel.height + skip;

            height = width;

            this._previewBorder.setPosition(x, y);
            this._previewBorder.setScale(width, height);

            x += 2; y += 2;
            width -= 4; height -= 4;

            this._previewBackground.setPosition(x, y);
            this._previewBackground.setScale(width, height);

            x += 2; y += 2;
            width -= 4; height -= 4;

            if (this._previewImage !== null) {
                this._previewImage.setPosition(x, y);

                const needsResize = this._previewImage.width > width || this._previewImage.height > height;
                //console.log("Needs resize: " + needsResize);
                if (needsResize) {
                    let aspect = 1;
                    if (this._previewImage.width >= this._previewImage.height) {
                        aspect = width / this._previewImage.width;
                    }
                    else if (this._previewImage.height > this._previewImage.width) {
                        aspect = height / this._previewImage.height;
                    }

                    this._previewImage.setScale(aspect, aspect);
                }
            }
        }
    }

    SetVisibility(value) {
        this._visible = value;
        this._previewBorder.setActive(value).setVisible(value);
        this._previewBackground.setActive(value).setVisible(value);
        this._backgroundSprite.setActive(value).setVisible(value);
        this._spriteSheetTextField.SetVisibility(value);
        this._atlasTextField.SetVisibility(value);
        this._spriteSheetLabel.setActive(value).setVisible(value);
        this._altasLabel.setActive(value).setVisible(value);
        this._previewLabel.setActive(value).setVisible(value);
        this._browseSpriteSheet.SetVisibility(value);
        this._atlasButton.SetVisibility(value);

        if (this._previewImage != null) {
            this._previewImage.setActive(value).setVisible(value);
        }
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
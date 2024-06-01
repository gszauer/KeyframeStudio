import UIConstants from './UIConstants.js'

export default class UIEditorBar {
    _scene = null;

    _y = 0;
    _width = 0;
    _height = 0;
    
    _backgroundSprite = null;
    _seperatorSprite = null;

    constructor(scene) {
        this._scene = scene;
        const self = this;

        self._seperatorSprite = scene.add.sprite(0, 0, UIConstants.Atlas, UIConstants.Solid);
        self._seperatorSprite.setDepth(UIConstants.WidgetLayer);
        self._seperatorSprite.setOrigin(0, 0);

        self._backgroundSprite = scene.add.sprite(0, 0, UIConstants.Atlas, UIConstants.Solid);
        self._backgroundSprite.setDepth(UIConstants.WidgetLayer);
        self._backgroundSprite.setOrigin(0, 0);
    }
   
    Layout() {
        const self = this;
       
        self._width = self._scene.scale.width;
        self._height = UIConstants.Sizes.EditorBarHeight;
        self._y = UIConstants.Sizes.TopMenuHeight;

        self._seperatorSprite.setTint(UIConstants.Colors.BorderDecorative);
        self._seperatorSprite.setPosition(0, self._y);
        self._seperatorSprite.setScale(self._width, self._height);

        const half = UIConstants.Sizes.EditorBarSeperatorHeight;
        self._backgroundSprite.setTint(UIConstants.Colors.BackgroundLayer1);
        self._backgroundSprite.setPosition(0, self._y + half);
        self._backgroundSprite.setScale(self._width, self._height - half * 2);

        const left = 0;
        const top = self._y + half;
        const right = left + self._width;
        const bottom = self._y + self._height - half * 2;
        // TODO: Layout children in the above rectangle
    }

    Destroy() {
        // TODO
    }
}
import UIGlobals from './UIGlobals.js'

export default class UIImageButton {
    _scene = null;
    _callback = null; // callback(iconName: string)
    _iconName = null;

    _borderSprite = null;
    _backgroundSprite = null;
    _iconSprite = null;

    constructor(scene, iconName = "", callback = null) {
        this._scene = scene;
        this._callback = callback;
        this._iconName = iconName;

        const self = this;

        this._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._borderSprite.setDepth(UIGlobals.WidgetLayer);
        this._borderSprite.setOrigin(0.5, 0.5);

        this._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        this._backgroundSprite.setOrigin(0.5, 0.5);

        this._iconSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, iconName);
        this._iconSprite.setDepth(UIGlobals.WidgetLayer);
        this._iconSprite.setOrigin(0.5, 0.5);
        const iconScaleX =  UIGlobals.Sizes.ToolboxButtonIconSize  /  this._iconSprite.width ;
        const iconScaleY =  UIGlobals.Sizes.ToolboxButtonIconSize /  this._iconSprite.height;
        this._iconSprite.setScale(iconScaleX, iconScaleY);

        this._backgroundSprite.setInteractive();
        this._backgroundSprite.on("pointerover", function (pointer, localX, localY, event) {
            if (UIGlobals.Active == null) {
                UIGlobals.Hot = self._backgroundSprite;
            }

            self.UpdateColors();
        });
        this._backgroundSprite.on("pointerout", function (pointer, event) {
            if (UIGlobals.Hot == self._backgroundSprite) {
                UIGlobals.Hot = null;
            }

            self.UpdateColors();
        });
        this._backgroundSprite.on("pointerdown", function (pointer, localX, localY, event) {
            UIGlobals.Active = self._backgroundSprite;
            UIGlobals.Hot = self._backgroundSprite;

            self.UpdateColors();
        });

        scene.input.on("pointerup", function(pointer, currentlyOver) {
            if (UIGlobals.Active != null && UIGlobals.Active == self._backgroundSprite) {
                let left = self._backgroundSprite.x - self._backgroundSprite.scaleX * 0.5;
                let right = left + self._backgroundSprite.scaleX;
                let top = self._backgroundSprite.y - self._backgroundSprite.scaleY * 0.5;
                let bottom = top + self._backgroundSprite.scaleY;
    
                if (pointer.x >= left && pointer.x <= right && pointer.y >= top && pointer.y <= bottom) {
                    if (self._callback != null) {
                        self._callback(self._iconName);
                    }
                }
                
                UIGlobals.Active = null;
            }
            self.UpdateColors();
        });
    }

    UpdateColors() {
        let background = UIGlobals.Colors.BackgroundLayer1;
        let icon = UIGlobals.Colors.IconSub;
       
        if (UIGlobals.Active == this._backgroundSprite) {
            background = UIGlobals.Colors.BackgroundLayer0;
        }
        else if (UIGlobals.Hot == this._backgroundSprite) {
            background = UIGlobals.Colors.BackgroundLayer1AndAHalf;
            icon = UIGlobals.Colors.Icon;
        }
        
        this._borderSprite.setTint(UIGlobals.Colors.BackgroundLayer2);
        this._backgroundSprite.setTint(background);
        this._iconSprite.setTint(icon);
    }

    Layout(x, y, width, height) {
        const bordeer = UIGlobals.Sizes.ImageButtonBorderSize;
        const margiin = UIGlobals.Sizes.ImageButtonMarginSize;
 
        this._borderSprite.setPosition(x, y);
        this._borderSprite.setScale(width, height);

        width -= bordeer * 2;
        height -= bordeer * 2;
 
         this._backgroundSprite.setPosition(x, y);
         this._backgroundSprite.setScale(width, height);

         width -= margiin * 2;
         height -= margiin * 2;
 
         this._iconSprite.setPosition(x, y);
         this._iconSprite.setScale(width / this._iconSprite.width, height / this._iconSprite.height);
 
         this.UpdateColors();
     }

     SetVisibility(value) {
        this._borderSprite.setActive(value).setVisible(value);
        this._backgroundSprite.setActive(value).setVisible(value);
        this._iconSprite.setActive(value).setVisible(value);
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }
}
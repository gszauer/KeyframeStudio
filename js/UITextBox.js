import UIGlobals from './UIGlobals.js'

export default class UITextBox {
    _scene = null;
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;
    _defaultWidth = 150;

    _text = ""; // always a string
    _bitmapText = null; // phaser bitmap text object
    _borderSprite = null;
    _backgroundSprite = null;
    _clickHandlerSprite = null;

    _rextextedit = null;

    onTextChanged = null; // Callback

    constructor(scene, text = "", defaultWidth = 0) {
        this._scene = scene;
        const self = this;
        self._defaultWidth = defaultWidth;
        if (defaultWidth == 0) {
            self._defaultWidth = 150;
        }

        self._borderSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._borderSprite.setDepth(UIGlobals.WidgetLayer);
        self._borderSprite.setOrigin(0, 0);

        self._backgroundSprite = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        self._backgroundSprite.setDepth(UIGlobals.WidgetLayer);
        self._backgroundSprite.setOrigin(0, 0);

        self._bitmapText = scene.add.bitmapText(0, 0, UIGlobals.Font100, name);
        self._bitmapText.setDepth(UIGlobals.WidgetLayer);
        self._bitmapText.text = text;
        self._text = text;
        
        
        /*Object.defineProperty(self._bitmapText, 'x', { get: function() { 
            return self._x + UIGlobals.Sizes.TextBoxBorderSize + UIGlobals.Sizes.TextboxTextMargin;
        }});
        Object.defineProperty(self._bitmapText, 'y', { get: function() { 
            return self._y + UIGlobals.Sizes.TextBoxBorderSize; 
        }});*/
        Object.defineProperty(self._bitmapText, 'width', { get: function() { 
            return self._width - UIGlobals.Sizes.TextboxTextMargin - UIGlobals.Sizes.TextBoxBorderSize * 2 - 1;
        }});
        Object.defineProperty(self._bitmapText, 'height', { get: function() { 
            return self._height - UIGlobals.Sizes.TextBoxBorderSize * 2 - 1 - 3; // -3 matches css border
        }});

        /*const getTextBounds = self._bitmapText.getTextBounds.bind(self._bitmapText);
        Object.defineProperty(self._bitmapText, 'getTextBounds', 
        self._bitmapText.getTextBounds = function (round) {
            const bounds = getTextBounds(round);
            bounds.global.x = bounds.local.x = self._x + UIGlobals.Sizes.TextBoxBorderSize + UIGlobals.Sizes.TextboxTextMargin;
            bounds.global.y = bounds.local.y = self._y + UIGlobals.Sizes.TextBoxBorderSize; 
            bounds.global.width = bounds.local.width = self._width - UIGlobals.Sizes.TextboxTextMargin - UIGlobals.Sizes.TextBoxBorderSize * 2 - 1;
            bounds.global.height = bounds.local.height = self._height - UIGlobals.Sizes.TextBoxBorderSize * 2 - 1;
            return bounds;
        });*/

        // For rext text input
        self._bitmapText.style = 
        {
            "backgroundColor": "#262626",
            "backgroundColor2": null,
            "backgroundHorizontalGradient": true,
            "backgroundStrokeColor": null,
            "backgroundStrokeLineWidth": 2,
            "backgroundCornerRadius": 0,
            "backgroundCornerIteration": null,
            "fontFamily": "Arial",
            "fontSize": "15px",
            "fontStyle": "",
            "color": "#ebebeb",
            "stroke": "#fff",
            "strokeThickness": 0,
            "shadowOffsetX": 0,
            "shadowOffsetY": 0,
            "shadowColor": "#000",
            "shadowBlur": 0,
            "shadowStroke": false,
            "shadowFill": false,
            "underlineColor": "#000",
            "underlineThickness": 0,
            "underlineOffset": 0,
            "strikethroughColor": "#000",
            "strikethroughThickness": 0,
            "strikethroughOffset": 0,
            "halign": "left",
            "valign": "center",
            "maxLines": 0,
            "fixedWidth": 200,
            "fixedHeight": 24,
            "resolution": 1,
            "lineSpacing": 0,
            "xOffset": 0,
            "rtl": false,
            "baselineX": 1.2,
            "baselineY": 1.4,
            "wrapMode": 0,
            "wrapWidth": 0,
            "wrapCallback": null,
            "wrapCallbackScope": null,
            "metrics": {
                "ascent": 12.24365234375,
                "descent": 3.2041015625,
                "fontSize": 15.44775390625
            }
        };

        self._bitmapText.padding = 
        {
            "left": 0,
            "right": 0,
            "top": 0,
            "bottom": 0
        };

        
    }

    UpdateColors() {
        let borderTint = UIGlobals.Colors.ElementBorderTintIdle;
        let backgroundTint = UIGlobals.Colors.BackgroundLayer1;

        if (UIGlobals.Hot == this._borderSprite) {
            borderTint = UIGlobals.Colors.ElementBorderTintHot;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }
        if (UIGlobals.Active == this._borderSprite) {
            borderTint = UIGlobals.Colors.ElementBorderTintActive;
            backgroundTint = UIGlobals.Colors.BackgroundLayer2;
        }

        this._backgroundSprite.setTint(backgroundTint);
        this._borderSprite.setTint(borderTint);
        this._bitmapText.setTint(borderTint);
    }

    _MakeSureTexTextEditorExists() {
        if (this._rextextedit == null) {
            const rextexteditplugin = this._scene.game.plugins.get('rextexteditplugin');
            this._rextextedit = rextexteditplugin.add(this._bitmapText, {
                type: 'text',
                enterClose: true,
                selectAll: true,
            
                onOpen: function (textObject) {
                    console.log('Open text editor');
                },
                onTextChanged: function (textObject, text) {
                    textObject.text = text;
                    console.log(`Text: ${text}`);
                },
                onClose: function (textObject) {
                    console.log('Close text editor');
                },
            });
        }
    }
    
    Layout(x, y, width = 0) {
        const self = this;
        
        self._x = x;
        self._y = y;
        self._width = width;
        if (width == 0) {
            self._width = self._defaultWidth;
            width = self._defaultWidth;
        }
        const height = self._height = UIGlobals.Sizes.TextboxHeight;
        const border = UIGlobals.Sizes.TextBoxBorderSize;
        const margin = UIGlobals.Sizes.TextboxTextMargin;

        self._borderSprite.setPosition(x, y);
        self._borderSprite.setScale(width, height);

        self._backgroundSprite.setPosition(x + border, y + border);
        self._backgroundSprite.setScale(width - border * 2, height - border * 2);

        self._bitmapText.setPosition(x + border + margin, y + border);
        self._bitmapText.setMaxWidth(width);

        self._bitmapText.style.fixedWidth = width;
        self._bitmapText.style.fixedHeight = height;

        const hitLeft = x + border + margin;
        const hitTop = y + border; 
        const hitWidth = width - margin - border * 2 - 1;
        const hitHeight = height - border * 2 - 1;
        
        self._scene.input.setHitArea(self._bitmapText, 
            new Phaser.Geom.Rectangle(hitLeft, hitTop, hitWidth, hitHeight),
            (hitArea, x, y, gameObject) => {
                const _hitWidth = self._width - UIGlobals.Sizes.TextboxTextMargin - UIGlobals.Sizes.TextBoxBorderSize * 2 - 1;
                const _hitHeight = self._height - UIGlobals.Sizes.TextBoxBorderSize * 2 - 1;
                return x >= 0 && y >= 0 && x <= _hitWidth && y <= _hitHeight;
            }
        );

        /*let styleText = JSON.stringify( self._bitmapText, null, 4);
        console.log("Bitmap Text:\n" + styleText);
        styleText = JSON.stringify( self._borderSprite, null, 4);
        console.log("Border Sprite:\n" + styleText);*/

        self._MakeSureTexTextEditorExists();
        self.UpdateColors();
    }
}
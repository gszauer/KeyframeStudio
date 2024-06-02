import UITopMenu from './UITopMenu.js'
import UIPopupMenu from './UIPopupMenu.js'
import UIConstants from './UIConstants.js'
import UIEditorBar from './UIEditorBar.js'
import UIDropdownBox from './UIDropdownBox.js'

export default class Application extends Phaser.Scene {
    _topMenu = null;
    _editorBar = null;

    _atlas = null;

    constructor() {
        super('Application');
    }

    preload() {
        const scene = this;

        this._atlas = scene.load.atlas(UIConstants.Atlas, 'img/atlas.png', 'img/atlas.json');
        
        this.load.xml(UIConstants.Font50 + ".xml", "img/" + UIConstants.Font50 + ".xml");
        this.load.xml(UIConstants.Font75 + ".xml", "img/" + UIConstants.Font75 + ".xml");
        this.load.xml(UIConstants.Font100 + ".xml", "img/" + UIConstants.Font100 + ".xml");
        this.load.xml(UIConstants.Font200 + ".xml", "img/" + UIConstants.Font200 + ".xml");
        this.load.xml(UIConstants.Font300 + ".xml", "img/" + UIConstants.Font300 + ".xml");
        this.load.xml(UIConstants.Font400 + ".xml", "img/" + UIConstants.Font400 + ".xml");
        this.load.xml(UIConstants.Font500 + ".xml", "img/" + UIConstants.Font500 + ".xml");
    }

    create() {
        const self = this;

        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIConstants.Font15 , UIConstants.Atlas, UIConstants.Font15  + ".png", UIConstants.Font15  + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIConstants.Font75 , UIConstants.Atlas, UIConstants.Font75  + ".png", UIConstants.Font75  + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIConstants.Font100, UIConstants.Atlas, UIConstants.Font100 + ".png", UIConstants.Font100 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIConstants.Font200, UIConstants.Atlas, UIConstants.Font200 + ".png", UIConstants.Font200 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIConstants.Font300, UIConstants.Atlas, UIConstants.Font300 + ".png", UIConstants.Font300 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIConstants.Font400, UIConstants.Atlas, UIConstants.Font400 + ".png", UIConstants.Font400 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIConstants.Font500, UIConstants.Atlas, UIConstants.Font500 + ".png", UIConstants.Font500 + ".xml");

        self._topMenu = new UITopMenu(self);
        self._editorBar = new UIEditorBar(self);

        const fileMenu = new UIPopupMenu(self);
        self._topMenu.Add("File", fileMenu);
        fileMenu.Add("New", null);
        fileMenu.Add("Open", null);
        fileMenu.Add("Save", null);
        fileMenu.Add("", null);
        fileMenu.Add("Import Asset", null);
        fileMenu.Add("", null);
        fileMenu.Add("Load Sample Project", null);

        const editMenu = new UIPopupMenu(self);
        self._topMenu.Add("Edit", editMenu);
        editMenu.Add("Undo", null);
        editMenu.Add("Redo", null);
        editMenu.Add("", null);
        editMenu.Add("Cut", null);
        editMenu.Add("Copy", null);
        editMenu.Add("Paste", null);

        const sceneMenu = new UIPopupMenu(self);
        self._topMenu.Add("Scene", sceneMenu);
        sceneMenu.Add("New Node", null);

        const animation = new UIPopupMenu(self);
        self._topMenu.Add("Animation", animation);
        animation.Add("New Animation", null);
        animation.Add("", null);
        animation.Add("Animate Draw Order Track", null);
        animation.Add("Animate Position Track", null);
        animation.Add("Animate Rotation Track", null);
        animation.Add("Animate Scale Track", null);
        animation.Add("Animate Tint Track", null);

        const helpMenu = new UIPopupMenu(self);
        self._topMenu.Add("Help", helpMenu);
        helpMenu.Add("About", null);
        helpMenu.Add("Documentation", null);
        helpMenu.Add("Source Code", null);

        const testDropdown = new UIDropdownBox(this, null, 300);
        const testPopup = new UIPopupMenu(self);
        testPopup.Add("Bicubic", null);
        testPopup.Add("Bilinear", null);
        testPopup.Add("Trilinear", null);
        testPopup.Add("Tricubic", null);
        testDropdown.SetMenu(testPopup);

        testDropdown.Layout(100, 39, 240);
        self.Layout();

        self.scale.on('resize', function(gameSize, baseSize, displaySize, previousWidth, previousHeight) {
            self.Layout();
        });
    }

    Destroy() {
        this._topMenu.Destroy();
        this._topMenu = null;
    }

    Layout() {
        this._topMenu.Layout();
        this._editorBar.Layout();
    }
}

window.addEventListener('load', () => {
    const config = {
        width: 800, 
        height: 600,
        backgroundColor: UIConstants.Colors.BackgroundLayer0,
        type: Phaser.AUTO,
        parent: 'phaser-game',
        scene: [Application],
        scale: {
            mode: Phaser.Scale.RESIZE,
            //autoCenter: Phaser.Scale.CENTER_BOTH,
        },
    }
   
    const game = new Phaser.Game(config);
});
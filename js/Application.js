import UIMenu from './UIMenu.js'
import UIPopup from './UIPopup.js'
import UIGlobals from './UIGlobals.js'
import UIToggle from './UIToggle.js'
import UIEditorBar from './UIEditorBar.js'
import UIDropdown from './UIDropdown.js'

export default class Application extends Phaser.Scene {
    _topMenu = null;
    _editorBar = null;

    _atlas = null;

    constructor() {
        super('Application');
    }

    preload() {
        const scene = this;

        this._atlas = scene.load.atlas(UIGlobals.Atlas, 'img/atlas.png', 'img/atlas.json');
        
        this.load.xml(UIGlobals.Font50 + ".xml", "img/" + UIGlobals.Font50 + ".xml");
        this.load.xml(UIGlobals.Font75 + ".xml", "img/" + UIGlobals.Font75 + ".xml");
        this.load.xml(UIGlobals.Font100 + ".xml", "img/" + UIGlobals.Font100 + ".xml");
        this.load.xml(UIGlobals.Font200 + ".xml", "img/" + UIGlobals.Font200 + ".xml");
        this.load.xml(UIGlobals.Font300 + ".xml", "img/" + UIGlobals.Font300 + ".xml");
        this.load.xml(UIGlobals.Font400 + ".xml", "img/" + UIGlobals.Font400 + ".xml");
        this.load.xml(UIGlobals.Font500 + ".xml", "img/" + UIGlobals.Font500 + ".xml");
    }

    create() {
        const self = this;

        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font15 , UIGlobals.Atlas, UIGlobals.Font15  + ".png", UIGlobals.Font15  + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font75 , UIGlobals.Atlas, UIGlobals.Font75  + ".png", UIGlobals.Font75  + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font100, UIGlobals.Atlas, UIGlobals.Font100 + ".png", UIGlobals.Font100 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font200, UIGlobals.Atlas, UIGlobals.Font200 + ".png", UIGlobals.Font200 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font300, UIGlobals.Atlas, UIGlobals.Font300 + ".png", UIGlobals.Font300 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font400, UIGlobals.Atlas, UIGlobals.Font400 + ".png", UIGlobals.Font400 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font500, UIGlobals.Atlas, UIGlobals.Font500 + ".png", UIGlobals.Font500 + ".xml");

        self._topMenu = new UIMenu(self);
        self._editorBar = new UIEditorBar(self);

        const fileMenu = new UIPopup(self);
        self._topMenu.Add("File", fileMenu);
        fileMenu.Add("New", null);
        fileMenu.Add("Open", null);
        fileMenu.Add("Save", null);
        fileMenu.Add("", null);
        fileMenu.Add("Import Asset", null);
        fileMenu.Add("", null);
        fileMenu.Add("Load Sample Project", null);

        const editMenu = new UIPopup(self);
        self._topMenu.Add("Edit", editMenu);
        editMenu.Add("Undo", null);
        editMenu.Add("Redo", null);
        editMenu.Add("", null);
        editMenu.Add("Cut", null);
        editMenu.Add("Copy", null);
        editMenu.Add("Paste", null);

        const sceneMenu = new UIPopup(self);
        self._topMenu.Add("Scene", sceneMenu);
        sceneMenu.Add("New Node", null);

        const animation = new UIPopup(self);
        self._topMenu.Add("Animation", animation);
        animation.Add("New Animation", null);
        animation.Add("", null);
        animation.Add("Animate Draw Order Track", null);
        animation.Add("Animate Position Track", null);
        animation.Add("Animate Rotation Track", null);
        animation.Add("Animate Scale Track", null);
        animation.Add("Animate Tint Track", null);

        const helpMenu = new UIPopup(self);
        self._topMenu.Add("Help", helpMenu);
        helpMenu.Add("About", null);
        helpMenu.Add("Documentation", null);
        helpMenu.Add("Source Code", null);

        const testDropdown = new UIDropdown(this, null, 300);
        const testPopup = new UIPopup(self);
        testPopup.Add("Bicubic", null);
        testPopup.Add("Bilinear", null);
        testPopup.Add("Trilinear", null);
        testPopup.Add("Tricubic", null);
        testDropdown.SetMenu(testPopup);

        const testToggle = new UIToggle(this, "Foo");

        testDropdown.Layout(100, 39, 240);
        testToggle.Layout(360, 39 + 3);


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
        backgroundColor: UIGlobals.Colors.BackgroundLayer0,
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
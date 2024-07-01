import UIMenu from './UIMenu.js'
import UIPopup from './UIPopup.js'
import UIGlobals from './UIGlobals.js'
import UIToolBar from './UIToolBar.js'
import UIToolBox from './UIToolBox.js'
import UISplitView from './UISplitView.js'
import UITabView from './UITabView.js'
import InspectorView from './InspectorView.js'
import DrawOrderView from './DrawOrderView.js'
import AnimationsView from './AnimationsView.js'
import AssetsView from './AssetsView.js'
import HierarchyView from './HierarchyView.js'
import * as TextEditPlugin from './rextexteditplugin.js';
import XForm from './Transform.js'


export default class Application extends Phaser.Scene {
    /* Layout */
    _toolBar = null;
    _toolBox = null;
    _mainSplitter = null; // Need to call layout (it's recursive)

    /* rendering and plugins */
    _atlas = null;
    rextexteditplugin = null;

    constructor() {
        super('Application');
    }

    AddNewNode() {
        
    }

    preload() {
        const scene = this;
        this._atlas = scene.load.atlas(UIGlobals.Atlas, 'img/Atlas.png', 'img/Atlas.json');
        
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

        this.rextexteditplugin = this.game.plugins.get('rextexteditplugin');
        if (!this.rextexteditplugin) {
            console.error("Could not load rextexteditplugin");
        }

        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font50 , UIGlobals.Atlas, UIGlobals.Font50  + ".png", UIGlobals.Font50  + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font75 , UIGlobals.Atlas, UIGlobals.Font75  + ".png", UIGlobals.Font75  + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font100, UIGlobals.Atlas, UIGlobals.Font100 + ".png", UIGlobals.Font100 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font200, UIGlobals.Atlas, UIGlobals.Font200 + ".png", UIGlobals.Font200 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font300, UIGlobals.Atlas, UIGlobals.Font300 + ".png", UIGlobals.Font300 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font400, UIGlobals.Atlas, UIGlobals.Font400 + ".png", UIGlobals.Font400 + ".xml");
        Phaser.GameObjects.BitmapText.ParseFromAtlas(self, UIGlobals.Font500, UIGlobals.Atlas, UIGlobals.Font500 + ".png", UIGlobals.Font500 + ".xml");

        self._toolBar = new UIToolBar(self);
        self._toolBox = new UIToolBox(self);

        this._toolBox.AddTop(UIGlobals.IconMove, null);
        this._toolBox.AddTop(UIGlobals.IconRotate, null);
        this._toolBox.AddTop(UIGlobals.IconScale, null);
        this._toolBox.AddTop("", null);
        this._toolBox.AddTop(UIGlobals.IconHand, null);
        this._toolBox.AddTop(UIGlobals.IconZoomIn, null);
        this._toolBox.AddTop(UIGlobals.IconGrid, null);

        this._toolBox.AddBottom("IconHelp.png", null);
        this._toolBox.AddBottom("IconDownload.png", null);
        this._toolBox.AddBottom("IconUpload.png", null);
        this._toolBox.AddBottom("IconNew.png", null);

        this._mainSplitter = new UISplitView(this, null);
        this._mainSplitter._distance = 300;
        this._mainSplitter.pinLeft = false;

        const timelineSplitter = this._mainSplitter.a = new UISplitView(this, this._mainSplitter);
        timelineSplitter.horizontal = false;
        timelineSplitter._distance = 280;
        timelineSplitter.pinTop = false;

        const toolSplitter = this._mainSplitter.b = new UISplitView(this, this._mainSplitter);
        toolSplitter.horizontal = false;
        toolSplitter._distance = 430;

        const inspectorTabs = toolSplitter.a = new UITabView(this, toolSplitter.a);
        const inspectorView = new InspectorView(this,  inspectorTabs);
        inspectorTabs.Add("Inspector", inspectorView);
        inspectorTabs.Add("Draw Order", new DrawOrderView(this, inspectorTabs));

        const sceneTabs = toolSplitter.b = new UITabView(this, toolSplitter.b);
        const hierarchyView = new HierarchyView(this, sceneTabs)
        sceneTabs.Add("Hierarchy", hierarchyView);
        sceneTabs.Add("Assets", new AssetsView(this, sceneTabs));
        sceneTabs.Add("Animations", new AnimationsView(this, sceneTabs));

        hierarchyView.onSelectionChanged = (oldNode, newNode) => {
            inspectorView.FocusOn(newNode);
        };

        self.Layout();
        self.scale.on('resize', function(gameSize, baseSize, displaySize, previousWidth, previousHeight) {
            self.Layout();
        });
    }

    Layout() {
        this._toolBar.Layout();
        this._toolBox.Layout();

        const x = this._toolBox._width;
        const y = this._toolBar._height;
        const width = this.scale.width - x;
        const height = this.scale.height - y;
        this._mainSplitter.Layout(x, y, width, height);
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
        },
        dom: {
            createContainer: true
        },
        audio: {
            noAudio: true
        },
        roundPixels: true,
        plugins: {
            global: [
                {
                    key: 'rextexteditplugin',
                    plugin: rextexteditplugin,
                    start: true
                }
            ]
        }
    }
   
    const game = new Phaser.Game(config);
});
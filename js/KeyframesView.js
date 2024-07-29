import UIImageButton from './UIImageButton.js'
import UIGlobals from './UIGlobals.js'
import UIListBox from './UIListBox.js'
import Clip from './Animation.js'
import UISplitView from './UISplitView.js'
import UIView from './UIView.js'

export default class KeyframesView extends UIView {
    _splitter = null;

    _leftHeader = null;
    _rightHeader = null;

    leftMask = null;
    rightMask = null;

    clip = null;
    _trackBackgrounds = [];
    _trackLabels = [];
    _trackCount = 0;

    _frameNumberBackgrounds = [];

    constructor(scene, parent = null) {
        super(scene, parent);
        const self = this;

        this._splitter = new UISplitView(scene, this);
        this._splitter.pinnedMinSize = 200;
        this._splitter.onResize = (distance) => { self.Layout(); }

        this._leftHeader = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._leftHeader.setDepth(UIGlobals.WidgetLayer);
        this._leftHeader.setOrigin(0, 0);

        this._rightHeader = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
        this._rightHeader.setDepth(UIGlobals.WidgetLayer);
        this._rightHeader.setOrigin(0, 0);
    }

    UpdateColors() {
        this._leftHeader.setTint(0x00ff00);
        this._rightHeader.setTint(UIGlobals.Colors.BackgroundLayer1);
        this._splitter.UpdateColors();
    }

    Layout(x, y, width, height) {
        if (x === undefined) { x = this._x; }
        if (y === undefined) { y = this._y; }
        if (width === undefined) { width = this._width; }
        if (height === undefined) { height = this._height; }

        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        
        super.Layout(x, y, width, height);

        const headerHeight = UIGlobals.Sizes.KeyframeViewHeaderHeight;
        const rowHeight = 26;

        this._splitter.Layout(x, y, width, height);
        width = this._splitter.distance;

        { // Left
            { // Header
                this._leftHeader.setPosition(x, y);
                this._leftHeader.setScale(width, headerHeight);
            }
            y += headerHeight;
            height -= headerHeight;

            if (this.clip !== null) {
                const trackCount = this._trackCount;
                for (let i = 0; i < trackCount; ++i) {
                    this._trackLabels[i].setPosition(x, y);
                    this._trackBackgrounds[i].setPosition(this._x, y);
                    this._trackBackgrounds[i].setScale(this._width, rowHeight);
                    y += rowHeight;//this._trackLabels[i].height;
                }
            }
        }
        
        x = this._x + width;
        y = this._y;
        width = this._width - width;
        height = this._height;

        {
            { // Header
                this._rightHeader.setPosition(x, y);
                this._rightHeader.setScale(width, headerHeight);

                const frameWidth = 10;
                const backupX = x;

                const clip = this.clip;
                if (clip !== null) {
                    const frameCount = clip.frameCount;
                    for (let i = 0; i < frameCount; ++i) {
                        this._frameNumberBackgrounds[i].setPosition(x, y);
                        this._frameNumberBackgrounds[i].setScale(frameWidth, headerHeight);
                        x += frameWidth;
                    }
                }
                x = backupX;
            }
            y += headerHeight;
            height -= headerHeight;
        }
    }

    SetVisibility(value) {
        this._leftHeader.setActive(value).setVisible(value);
        this._rightHeader.setActive(value).setVisible(value);
        this._splitter.SetVisibility(value);
        // TODO

        if (!value) {
            throw new Error("Can't hide keyframe view because of the way labels are recycled");
        }
    }

    Hide() {
        this.SetVisibility(false);
    }

    Show() {
        this.SetVisibility(true);
    }

    FocusOn(clip) {
        this.clip = clip;
        const scene = this._scene;
        const tint1 = UIGlobals.Colors.BackgroundLayer1AndAHalf;
        const tint2 = UIGlobals.Colors.BackgroundLayer1;
        
        if (clip == null) {
            console.log("" +Date.now() + "  Focus on null");
        }
        else {
            console.log("" + Date.now() + "  Focus on: " + clip.name);
        }

        { // Clear old
            const numTracks = this._trackLabels.length;
            for (let i = 0; i < numTracks; ++i) {
                this._trackBackgrounds[i].setActive(false).setVisible(false);
                this._trackLabels[i].setActive(false).setVisible(false);
            }
            const numFrames = this._frameNumberBackgrounds.length;
            for (let i = 0; i < numFrames; ++i) {
                this._frameNumberBackgrounds[i].setActive(false).setVisible(false);
            }
        }

        if (clip !== null) { // Make sure there are enough tracks
            const numTracks = clip.tracks.length;
            if (numTracks > this._trackCount) {
                for (let i = this._trackCount; i < numTracks; ++i) {
                    const background = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
                    background.setDepth(UIGlobals.WidgetLayer);
                    background.setOrigin(0, 0);
                    if (i % 2 == 0) { background.setTint(tint1); }
                    else { background.setTint(tint2); }
                    this._trackBackgrounds.push(background);

                    const label = scene.add.bitmapText(0, 0, UIGlobals.Font100);
                    label.setDepth(UIGlobals.WidgetLayer);
                    label.setTint(UIGlobals.Colors.Icon);
                    this._trackLabels.push(label);
                }
            }
            this._trackCount = numTracks;
            for (let i = 0; i < numTracks; ++i) {
                this._trackLabels[i].setActive(true).setVisible(true);
                this._trackBackgrounds[i].setActive(true).setVisible(true);
            }
        }

        if (clip !== null) { // Update labels
            const numTracks = this._trackCount;
            const tracks = clip.tracks;

            for (let i = 0; i < numTracks; ++i) {
                this._trackLabels[i].text = tracks[i].target.name + "(" + tracks[i].type + ")";
            }
        }

        if (clip !== null) { // Make sure there are enough frames
            const frameCount = clip.frameCount;
            
            if (this._frameNumberBackgrounds.length < frameCount) {
                for (let i = this._frameNumberBackgrounds.length; i < frameCount; ++i) {
                    const frame = scene.add.sprite(0, 0, UIGlobals.Atlas, UIGlobals.Solid);
                    frame.setDepth(UIGlobals.WidgetLayer);
                    frame.setOrigin(0, 0);
                    if (i % 2 == 0) { frame.setTint(tint1); }
                    else { frame.setTint(tint2); }
                    this._frameNumberBackgrounds.push(frame);
                }
            }
            for (let i = 0; i < frameCount; ++i) {
                this._frameNumberBackgrounds[i].setActive(true).setVisible(true);
            }
        }

        this.Layout();
        this.UpdateColors();
    }
}
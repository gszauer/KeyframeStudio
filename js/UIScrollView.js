import UIView from './UIView.js'
import UIGlobals from './UIGlobals.js'

// https://csdgn.org/article/scrollbar
export default class UIScrollView extends UIView {

    container = null; // Phaser container
    
    _backgroundSprite = null;
    _handleBgSprite = null;
    _upBgSprite = null;
    _downBgSprite = null;
    _upIconSprite = null;
    _downIconSprite = null;

    constructor(scene, parent = null) {
        super(scene, parent);
    }

    UpdateColors() {

    }

    Layout(x, y, width, height) {
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }

        super.Layout(x, y, width, height);
    }

    Hide() {
        
    }

    Show() {
        
    }
}
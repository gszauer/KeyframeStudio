
export class UIFooterButton { 
    constructor(name, icon, callback) {

    }
}

export default class UIFooter {
    _background = null;
    _buttons = [];

    constructor() {
        
    }

    Add(name, icon, callback) {
        const button = new UIFooterButton(name, icon, callback);
        this._buttons.push(button);
    }

    Layout(x, y, w, h) {

    }
}
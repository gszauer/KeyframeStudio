import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UITextBox from './UITextBox.js'
import UIScrollView from './UIScrollView.js'

export default class InspectorView extends UIView {
    // TODO: Remove scroll area! Adding a mask in a mask is a no-go
    // which makes the inspector view kind of broken right now
    _scrollArea = null;
    _container = null;

    _transformLabel = null;
    _nameLabel = null;
    _nameTextField = null;

    constructor(scene, parent = null) {
        super(scene, parent);

        this._scrollArea = new UIScrollView(scene, this);
        this._scrollArea.showHorizontal = false;
        this._container = this._scrollArea.container;

        this._transformLabel = scene.add.bitmapText(0, 0, UIGlobals.Font400, name);
        this._transformLabel.setDepth(UIGlobals.WidgetLayer);
        this._transformLabel.text = "Transform";

        this._nameLabel = scene.add.bitmapText(0, 0, UIGlobals.Font50, name);
        this._nameLabel.setDepth(UIGlobals.WidgetLayer);
        this._nameLabel.text = "Name";

        this._nameTextField = new UITextBox(scene, "None", null, 400);

        this._container.add(this._transformLabel);
        this._container.add(this._nameLabel);
        //this._nameTextField.AddToContainer(this._container);
   }

    UpdateColors() {
        this._scrollArea.UpdateColors();
        // TODO
    }

    Layout(x, y, width, height) {
        super.Layout(x, y, width, height);
        if (width < 0) { width = 0; }
        if (height < 0) { height = 0; }
        
        this._x = x;
        this._y = y;
        this._width = width;
        this._height = height;

        const margin = UIGlobals.Sizes.InspectorTitleMargin;
        const skip = UIGlobals.Sizes.InspectorTitleSkip;

        x = margin;
        y = 0;

        this._transformLabel.setPosition(x, y);

        x += margin;
        y = y + this._transformLabel.height + skip;

        this._nameLabel.setPosition(x, y);
        y = y + this._nameLabel.height + skip;

        this._nameTextField.Layout(x, y);


        this._scrollArea.Layout(this._x, this._y, this._width, this._height);
    }

    Hide() {
        this._scrollArea.Hide()
        //throw new Error("View hide method not implemented");
    }

    Show() {
        this._scrollArea.Show()
        //throw new Error("View hide method not implemented");
    }
}
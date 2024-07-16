import UIGlobals from './UIGlobals.js'
import UIView from './UIView.js'
import UIToolBarShelf  from './UIToolBarShelf.js';
import UITextBox from './UITextBox.js'
import UIDropdown from './UIDropdown.js'
import UIPopup from './UIPopup.js'
import UIToggle from './UIToggle.js'
import XForm from './Transform.js'
import PanShelf from './ShelfPan.js'

export default class ZoomShelf extends PanShelf {
    DragStart(pointer) {
        this._dragStart.x = pointer.worldX;
        this._dragStart.y = pointer.worldY;

        this._cameraStart.x = this._sceneView._cameraTransform.x;// * this._sceneView._cameraTransform.scaleX;
        this._cameraStart.y = this._sceneView._cameraTransform.y;// * this._sceneView._cameraTransform.scaleY;
        this._cameraStart.scaleX = this._sceneView._cameraTransform.scaleX;
        this._cameraStart.scaleY = this._sceneView._cameraTransform.scaleY;
    }

    Drag(pointer, dragX, dragY) {
        const deltaX = (pointer.worldX - this._dragStart.x) * 0.01;
        const deltaY = (pointer.worldY - this._dragStart.y) * 0.01;
        
        const scaleX = Math.abs(this._cameraStart.scaleX + deltaX);
        const scaleY = Math.abs(this._cameraStart.scaleY + deltaY);

        this._sceneView._cameraTransform.scaleX = scaleY;
        this._sceneView._cameraTransform.scaleY = scaleY;

        //this._sceneView._cameraTransform.x = this._cameraStart.x + (this._cameraStart.x * scaleY - this._cameraStart.x);
        //this._sceneView._cameraTransform.y = this._cameraStart.y + (this._cameraStart.y * scaleY - this._cameraStart.y);
 
        const truncateFloat = (str, digits) => {
            let re = new RegExp("(\\d+\\.\\d{" + digits + "})(\\d)"),
                m = str.toString().match(re);
            return m ? parseFloat(m[1]) : str.valueOf();
        };

        this._zoomInput.text = + truncateFloat("" + scaleY, 3);
        this._viewportXInput.text = truncateFloat("" + this._sceneView._cameraTransform.x, 3);
        this._viewportYInput.text = truncateFloat("" + this._sceneView._cameraTransform.y, 3);

        this._sceneView.Layout();
    }

    DragEnd(pointer) {

    }
}
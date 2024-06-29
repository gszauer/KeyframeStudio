
// https://github.com/phaserjs/phaser/blob/v3.80.0/src/gameobjects/components/Transform.js
// https://github.com/phaserjs/phaser/blob/master/src/gameobjects/components/TransformMatrix.js#L39

// Transforms are not stand-alone. The parent / child relationship comes from the owning UITreeNode
export default class XForm {
    _uiTreeNode = null;

    x = 0;
    y = 0;
    rotation = 0; // in radians
    scaleX = 1;
    scaleY = 1;
    uniform = false;

    constructor(treeNode = null) {
        this._uiTreeNode = treeNode;
        if (treeNode) {
            if (!treeNode._userData) {
                treeNode._userData = {
                    transform: this
                };
            }
            else {
                treeNode._userData.transform = this;
            }
        }
    }

    get localMatrix() {
        let result = new Mat3();
        result.setTRS(this.x, this.y, this.rotation, this.scaleX, this.scaleY);
        return result;
    }

    get worldMatrix() {
        let result = this.localMatrix;

        let parent = this.parent;
        if (!parent) { return result; }

        let parentMatrix = new Mat3();
        while(parent) {
            parentMatrix.setTRS(parent.x, parent.y, parent.rotation, parent.scaleX, parent.scaleY);
            parentMatrix.mult(result, result);
            parent = parent.parent;
        }

        return result;
    }

    get name() {
        if (this._uiTreeNode == null) {
            return null;
        }
        return this._uiTreeNode._name;
    }

    get parent() {
        if (this._uiTreeNode == null) {
            return null;
        }

        const parentUINode = this._uiTreeNode._parent;
        if (parentUINode == null) {
            return null;
        }

        return parentUINode._userData.transform;
    }

    get firstChild() {
        if (this._uiTreeNode == null) {
            return null;
        }

        const firstChildUINode = this._uiTreeNode._firstChild;
        if (firstChildUINode == null) {
            return null;
        }

        return firstChildUINode._userData.transform;
    }

    get nextSibling() {
        if (this._uiTreeNode == null) {
            return null;
        }

        const nextUISibling = this._uiTreeNode._nextSibling;
        if (nextUISibling == null) {
            return null;
        }

        return nextUISibling._userData.transform;
    }

    get uiTreeNode() {
        return this._uiTreeNode;
    }
}

export class Mat3 {
    /* | a | c | tx | *
     * | b | d | ty | *
     * | 0 | 0 | 1  | */

    static _a = 0;
    static _b = 1;
    static _c = 3;
    static _d = 4;
    static _x = 6;
    static _y = 7;
  
    matrix = new Float32Array(9);

    constructor(a, b, c, d, x, y) {
        if (a === undefined) { this.a = 1; } else { this.a = a; }
        if (b === undefined) { this.b = 0; } else { this.b = b; }
        if (c === undefined) { this.c = 0; } else { this.c = c; }
        if (d === undefined) { this.d = 1; } else { this.d = d; }
        if (x === undefined) { this.x = 0; } else { this.x = x; }
        if (y === undefined) { this.y = 0; } else { this.y = y; }
        this.matrix[2] = this.matrix[5] = 0;
        this.matrix[8] = 1;
    }

    setTRS(x, y, rotation /* radians*/, scaleX, scaleY) {
        let matrix = this.matrix;

        let _sin = Math.sin(rotation);
        let _cos = Math.cos(rotation);

        // Set Translation
        this.x = x;
        this.y = y;

        // Set Rotate and Scale
        this.a = _cos * scaleX;
        this.b = _sin * scaleX;
        this.c = -_sin * scaleY;
        this.d = _cos * scaleY;

        return this;
    }

    mult(rhs, out) {
        const matrix = this.matrix;
        const source = rhs.matrix;

        const localA = matrix[Mat3._a];
        const localB = matrix[Mat3._b];
        const localC = matrix[Mat3._c];
        const localD = matrix[Mat3._d];
        const localE = matrix[Mat3._x];
        const localF = matrix[Mat3._y];

        const sourceA = source[Mat3._a];
        const sourceB = source[Mat3._b];
        const sourceC = source[Mat3._c];
        const sourceD = source[Mat3._d];
        const sourceE = source[Mat3._x];
        const sourceF = source[Mat3._y];

        const destinationMatrix = (out === undefined) ? matrix : out.matrix;

        destinationMatrix[0] = (sourceA * localA) + (sourceB * localC);
        destinationMatrix[1] = (sourceA * localB) + (sourceB * localD);
        destinationMatrix[2] = (sourceC * localA) + (sourceD * localC);
        destinationMatrix[3] = (sourceC * localB) + (sourceD * localD);
        destinationMatrix[4] = (sourceE * localA) + (sourceF * localC) + localE;
        destinationMatrix[5] = (sourceE * localB) + (sourceF * localD) + localF;

        return destinationMatrix;
    }

    get a() {
        return this.matrix[0];
    }
    
    set a(v) {
        this.matrix[0] = v;
    }

    get b() {
        return this.matrix[1];
    }
    set b(v) {
        this.matrix[1] = v;
    }

    get c() {
        return this.matrix[3];
    }
    set c(V) {
        this.matrix[3] = v;
    }

    get d() {
        return this.matrix[4];
    }
    set d(v) {
        this.matrix[4] = v;
    }

    get x() {
        return this.matrix[6];
    }
    set x(v) {
        this.matrix[6] = v;
    }

    get y() {
        return this.matrix[7];
    }
    set y(v) {
        this.matrix[7] = v;
    }
    
}
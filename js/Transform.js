// https://github.com/phaserjs/phaser/blob/v3.80.0/src/gameobjects/components/Transform.js
// https://github.com/phaserjs/phaser/blob/master/src/gameobjects/components/TransformMatrix.js#L39

// Transforms are not stand-alone. The parent / child relationship comes from the owning UITreeNode
export default class XForm {
    _uiTreeNode = null; // Node that this transform is attached to

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

    Copy() {
        const result = new XForm(null);
        result._uiTreeNode = this._uiTreeNode;
        result.x = this.x;
        result.y = this.y;
        result.rotation = this.rotation;
        result.scaleX = this.scaleX;
        result.scaleY = this.scaleY;
        result.uniform = this.uniform;
        return result;
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

    get worldTransform() {
        const identity = () => {
            return {
                x: 0, y: 0,
                rotation: 0,
                scaleX: 1, scaleY: 1
            };
        }
        const worldXform = identity();

        for (let iter = this; iter != null; iter = iter.parent) {
            XForm.Mul(iter, worldXform, worldXform);
        }

        return worldXform;
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

    get degrees() {
        return this.rotation * 57.2958;
    }

    set degrees(valuue) {
        this.rotation = valuue * 0.0174533; 
    }

    static Right(xform) {
        const RotateClockwise = (_x, _y, radians) => {
            const cs = Math.cos(radians);
            const sn = Math.sin(radians);
            
            return {
                x: _x * cs - _y * sn,
                y: _x * sn + _y * cs
            };
        };
        return RotateClockwise(1, 0, xform.rotation);
    }

    static Up(xform) {
        const RotateClockwise = (_x, _y, radians) => {
            const cs = Math.cos(radians);
            const sn = Math.sin(radians);
            
            return {
                x: _x * cs - _y * sn,
                y: _x * sn + _y * cs
            };
        };
        return RotateClockwise(0, 1, xform.rotation);
    }

    static Inverse(xfrm) {
        const RotateClockwise = (_x, _y, radians) => {
            const cs = Math.cos(radians);
            const sn = Math.sin(radians);
            
            return {
                x: _x * cs - _y * sn,
                y: _x * sn + _y * cs
            };
        };

        const InvertAngle = (angle) => {
            return (angle + Math.PI) % (2 * Math.PI);
        }

        const result = {
            x: 0, y: 0,
            rotation: 0,
            scaleX: 1, scaleY: 1
        };

        result.rotation = InvertAngle(xfrm.rotation);
        
        if (Math.abs(xfrm.scaleX) > 0.00001) {
            result.scaleX = 1.0 / xfrm.scaleX;
        }
        if (Math.abs(xfrm.scaleY) > 0.00001) {
            result.scaleY = 1.0 / xfrm.scaleY;
        }

        result.x = (-xfrm.x) * result.scaleX;
        result.y = (-xfrm.y) * result.scaleY;

        const rotated = RotateClockwise(result.x, result.y, result.rotation);
        result.x = rotated.x;
        result.y = rotated.y;
        
        return result;
    }

    ApplyTransform(sprite, view = null) {
        const worldXform = this.worldTransform;
        XForm.Mul(view, worldXform, worldXform);

        sprite.setPosition(worldXform.x, worldXform.y);
        sprite.setRotation(worldXform.rotation);
        sprite.setScale(worldXform.scaleX, worldXform.scaleY);
    }

    static ApplyToPoint(xfrm, pnt, view = null) {
        const RotateClockwise = (_x, _y, radians) => {
            const cs = Math.cos(radians);
            const sn = Math.sin(radians);
            
            return {
                x: _x * cs - _y * sn,
                y: _x * sn + _y * cs
            };
        };

        const result = RotateClockwise(xfrm.scaleX * pnt.x, xfrm.scaleY * pnt.y, xfrm.rotation);
        result.x += xfrm.x;
        result.y += xfrm.y;
        return result;
    }

    static Mul(a /*parent*/, b /*transform*/, c /*out*/) {
        const RotateClockwise = (_x, _y, radians) => {
            const cs = Math.cos(radians);
            const sn = Math.sin(radians);
            
            return {
                x: _x * cs - _y * sn,
                y: _x * sn + _y * cs
            };
        };

        const MakeIdentity = () => {
            return {
                x: 0, y: 0,
                rotation: 0,
                scaleX: 1, scaleY: 1
            };
        };

        if (a === null || a === undefined) {
            a = MakeIdentity();
        }
        if (c === null || c === undefined) {
            c = MakeIdentity();
        }
        if (b === null || b === undefined) {
            b = MakeIdentity();
        }


        c.scaleX = a.scaleX * b.scaleX;
        c.scaleY = a.scaleY * b.scaleY;

        c.rotation = a.rotation + b.rotation;

        // parent scale times child position, rotated by parent rotation:
        const rotated = RotateClockwise(a.scaleX * b.x, a.scaleY * b.y, a.rotation)
        // combine positions
        c.x = a.x + rotated.x;
        c.y = a.y + rotated.y;

        return c;
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
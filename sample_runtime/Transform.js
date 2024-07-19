/*
get absolute() {
        const worldXform = {
            position: { x: 0, y: 0 },
            rotation: 0,
            scale: { x: 0, y: 0 }
        };

        for (let iter = this; iter != null; iter = iter.parent) {
            Transform.Mul(iter, worldXform, worldXform);
        }

        return worldXform;
    }
*/

export default class Transform {
    position = { x: 0, y: 0 };
    rotation = 0;
    scale = { x: 0, y: 0 };

    constructor(positionOrTransformOrUndefined, rotationOrUndefined, scaleOrUndefined) {
        if (positionOrTransformOrUndefined !== undefined) {
            if (positionOrTransformOrUndefined instanceof Transform) {
                this.position.x = positionOrTransformOrUndefined.position.x;
                this.position.y = positionOrTransformOrUndefined.position.y;
                this.rotation = positionOrTransformOrUndefined.rotation;
                this.scale.x = positionOrTransformOrUndefined.scale.x;
                this.scale.y = positionOrTransformOrUndefined.scale.y;
            }
            else {
                if (positionOrTransformOrUndefined.hasOwnProperty("position")) {
                    const position = positionOrTransformOrUndefined.position;
                    if (position.hasOwnProperty("x")) {
                        this.position.x = position.x;
                    }
                    if (position.hasOwnProperty("y")) {
                        this.position.y = position.y;
                    }
                }
                else {
                    if (positionOrTransformOrUndefined.hasOwnProperty("x")) {
                        this.position.x = positionOrTransformOrUndefined.x;
                    }
                    if (positionOrTransformOrUndefined.hasOwnProperty("y")) {
                        this.position.y = positionOrTransformOrUndefined.y;
                    }
                }

                if (positionOrTransformOrUndefined.hasOwnProperty("rotation")) {
                    this.rotation = positionOrTransformOrUndefined.rotation;
                }
                else if (positionOrTransformOrUndefined.hasOwnProperty("radians")) {
                    this.rotation = positionOrTransformOrUndefined.radians;
                }
                else if (positionOrTransformOrUndefined.hasOwnProperty("angles")) {
                    this.rotation = positionOrTransformOrUndefined.angles * 0.0174533;
                }
                else if (positionOrTransformOrUndefined.hasOwnProperty("degrees")) {
                    this.rotation = positionOrTransformOrUndefined.degrees * 0.0174533;
                }

                if (positionOrTransformOrUndefined.hasOwnProperty("scale")) {
                    const scale = positionOrTransformOrUndefined.scale;
                    if (scale.hasOwnProperty("x")) {
                        this.scale.x = scale.x;
                    }
                    if (scale.hasOwnProperty("y")) {
                        this.scale.y = scale.y;
                    }
                }
                else {
                    if (positionOrTransformOrUndefined.hasOwnProperty("scaleX")) {
                        this.scale.x = positionOrTransformOrUndefined.scaleX;
                    }
                    if (positionOrTransformOrUndefined.hasOwnProperty("scaleY")) {
                        this.scale.y = positionOrTransformOrUndefined.scaleY;
                    }
                }
            }
        }

        if (rotationOrUndefined !== undefined) {
            this.rotation = rotationOrUndefined;
        }

        if (scaleOrUndefined !== undefined) {
            if (scaleOrUndefined.hasOwnProperty("x")) {
                this.scale.x = scaleOrUndefined.x;
            }
            else if (scaleOrUndefined.hasOwnProperty("scaleX")) {
                this.scale.x = scaleOrUndefined.scaleX;
            }

            if (scaleOrUndefined.hasOwnProperty("y")) {
                this.scale.y = scaleOrUndefined.y;
            }
            else if (scaleOrUndefined.hasOwnProperty("scaleY")) {
                this.scale.y = scaleOrUndefined.scaleY;
            }
        }
    }

    get x() {
        return this.position.x;
    }

    set x(v) {
        this.position.x = v;
    }

    get y() {
        return this.position.y;
    }

    set y(v) {
        this.position.y = v;
    }

    get scaleX() {
        return this.scale.x;
    }

    set scaleX(v) {
        this.scale.x = v;
    }

    get scaleY() {
        return this.scale.y;
    }

    set scaleY(v) {
        this.scale.y = v;
    }

    get degrees() {
        return this.rotation * 57.2958;
    }

    set degrees(valuue) {
        this.rotation = valuue * 0.0174533; 
    }

    get angle() {
        return this.rotation * 57.2958;
    }

    set angle(valuue) {
        this.rotation = valuue * 0.0174533; 
    }

    get radians() {
        return this.rotation;
    }

    set radians(valuue) {
        this.rotation = valuue; 
    }

    ApplyToPoint(point) {
        return Transform.ApplyToPoint(this, point);
    }

    // Left off here
    static RotateClockwise (xOrPoint, yOrRadians, radiansOrUndefined) {
        let _x = xOrPoint;
        let _y = yOrRadians;
        let _radians = radiansOrUndefined;

        if (xOrPoint.hasOwnProperty("x") && xOrPoint.hasOwnProperty("y")) {
            _x = xOrPoint.x;
            _y = xOrPoint.y;
            _radians = yOrRadians;
        }

        const cs = Math.cos(_radians);
        const sn = Math.sin(_radians);
        
        return {
            x: _x * cs - _y * sn,
            y: _x * sn + _y * cs
        };
    };

    static RotateCounterClockwise (xOrPoint, yOrRadians, radiansOrUndefined) {
        let _x = xOrPoint;
        let _y = yOrRadians;
        let _radians = radiansOrUndefined;

        if (xOrPoint.hasOwnProperty("x") && xOrPoint.hasOwnProperty("y")) {
            _x = xOrPoint.x;
            _y = xOrPoint.y;
            _radians = yOrRadians;
        }
        
        const cs = Math.cos(_radians);
        const sn = Math.sin(_radians);
        
        return {
            x: _x * cs + _y * sn,
            y: -_x * sn + _y * cs
        };
    };

    static InvertAngle (angle) {
        return (angle + Math.PI) % (2 * Math.PI);
    }

    static EPSILON = 0.00001;

    static Inverse(xfrm) {
        const epsilon = Transform.EPSILON;
        const result = new Transform();
        result.rotation = Transform.InvertAngle(xfrm.rotation);
        
        if (xfrm.scaleX > epsilon || xfrm.scaleX < -epsilon) {
            result.scaleX = 1.0 / xfrm.scaleX;
        }
        if (xfrm.scaleY > epsilon || xfrm.scaleY < -epsilon) {
            result.scaleY = 1.0 / xfrm.scaleY;
        }

        result.x = (-xfrm.x) * result.scaleX;
        result.y = (-xfrm.y) * result.scaleY;

        const rotated = Transform.RotateClockwise(result.x, result.y, result.rotation);
        result.x = rotated.x;
        result.y = rotated.y;
        
        return result;
    }

    static ApplyToPoint(xfrm, pnt) {
        const result = Transform.RotateClockwise(xfrm.scaleX * pnt.x, xfrm.scaleY * pnt.y, xfrm.rotation);
        result.x += xfrm.x;
        result.y += xfrm.y;
        return result;
    }

    static Mul(a /*parent*/, b /*transform*/, c /*out*/) {
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
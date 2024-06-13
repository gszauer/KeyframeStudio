import ColorRGB from './ColorRGB.js'

export default class ColorHSV {
    h = 0; // Range of 0 to 359
    s = 0; // Range of 0 to 1
    v = 0; // Range of 0 to 1

    constructor(h = 0, s = 0, v = 0) {
        if (h instanceof ColorHSV) {
            this.h = h.h;
            this.s = s.s;
            this.v = v.v;
        }
        else if (h instanceof ColorRGB) {
            const hsv = r.hsv;
            this.h = rgb.h;
            this.s = rgb.s;
            this.v = rgb.v;
        }
        else {
            this.h = h;
            this.s = s;
            this.v = v;
        }

        this.normalize();
    }

    normalize() {
        if (this.s < 0) { this.s = 0; }
        if (this.v < 0) { this.v = 0; }

        if (this.s > 1) { this.s = 1; }
        if (this.v > 1) { this.v = 1; }

        while (this.h < 0) { this.h += 360; }
        while (this.h >= 360) { this.h -= 360; }
    }

    compare(other) {
        return other.rgb.compare(this.rgb);
    }

    lerp(target, t) {
        return new ColorHSV(
            this.h + (target.h - this.h) * t,
            this.s + (target.s - this.s) * t,
            this.v + (target.v - this.v) * t
        );
    }

    get rgb() {
        const out = new ColorRGB();
    
        if (this.s <= 0.0) {       // < is bogus, just shuts up warnings
            out.r = this.v;
            out.g = this.v;
            out.b = this.v;
            return out;
        }
        let hh = this.h;
        if (hh >= 360.0) hh = 0.0;
        hh /= 60.0;
        let i = Math.floor(hh);
        let ff = hh - i;
        let p = this.v * (1.0 - this.s);
        let q = this.v * (1.0 - (this.s * ff));
        let t = this.v * (1.0 - (this.s * (1.0 - ff)));
    
        switch (i) {
        case 0:
            out.r = this.v;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = this.v;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = this.v;
            out.b = t;
            break;
    
        case 3:
            out.r = p;
            out.g = q;
            out.b = this.v;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = this.v;
            break;
        case 5:
        default:
            out.r = this.v;
            out.g = p;
            out.b = q;
            break;
        }
        return out;
    }

    set rgb(value) {
        if (!(value instanceof ColorRGB)) {
            throw Error("value must be ColorRGB");
        }
        const hsv = value.hsv;
        this.h = hsv.h;
        this.s = hsv.s;
        this.v = hsv.v;
    }

    get color() {
        return this.rgb.color;
    }
}
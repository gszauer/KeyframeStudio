
import ColorHSV from './ColorHSV.js'

export default class ColorRGB {
    r = 0; // Range of 0 to 1
    g = 0; // Range of 0 to 1
    b = 0; // Range of 0 to 1

    constructor(r = 0, g = 0, b = 0) {
        if (r instanceof ColorRGB) {
            this.r = r.r;
            this.g = r.g;
            this.b = r.b;
        }
        else if (r instanceof ColorHSV) {
            const rgb = r.rgb;
            this.r = rgb.r;
            this.g = rgb.g;
            this.b = rgb.b;
        }
        else {
            this.r = r;
            this.g = g;
            this.b = b;
        }

        if (this.r < 0) { this.r = 0; }
        if (this.g < 0) { this.g = 0; }
        if (this.b < 0) { this.b = 0; }

        if (this.r > 1) { this.r = 1; }
        if (this.g > 1) { this.g = 1; }
        if (this.b > 1) { this.b = 1; }
    }

    normalize() {
        if (this.r < 0) { this.r = 0; }
        if (this.g < 0) { this.g = 0; }
        if (this.b < 0) { this.b = 0; }

        if (this.r > 1) { this.r = 1; }
        if (this.g > 1) { this.g = 1; }
        if (this.b > 1) { this.b = 1; }
    }

    compare(other) {
        if (!(other instanceof ColorRGB)) {
            return false;
        }

        const ar = Math.floor(this.r * 255.0);
        const ag = Math.floor(this.g * 255.0);
        const ab = Math.floor(this.b * 255.0);
    
        const br = Math.floor(other.r * 255.0);
        const bg = Math.floor(other.g * 255.0);
        const bb = Math.floor(other.b * 255.0);
    
        return ar == br && ab == bb && ag == bg;
    }

    lerp(target, t) {
        return new ColorRGB(
            this.r + (target.r - this.r) * t,
            this.g + (target.g - this.g) * t,
            this.b + (target.b - this.b) * t
        );
    }

    get hsv() {
        const out = new ColorHSV();

        let min = this.r < this.g ? this.r : this.g;
        min = min < this.b ? min : this.b;

        let max = this.r > this.g ? this.r : this.g;
        max = max > this.b ? max : this.b;

        let delta = max - min;

        out.v = max;                                // v
        if (delta < 0.00001) {
            out.s = 0;
            out.h = 0; // undefined, maybe nan?
            return out;
        }
        if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
            out.s = (delta / max);                  // s
        }
        else {
            // if max is 0, then r = g = b = 0              
            // s = 0, h is undefined
            out.s = 0.0;
            out.h = 0.0;                            // its now undefined
            return out;
        }
        if (this.r >= max) {                          // > is bogus, just keeps compilor happy
            out.h = (this.g - this.b) / delta;        // between yellow & magenta
        } 
        else {
            if (this.g >= max) {
                out.h = 2.0 + (this.b - this.r) / delta;  // between cyan & yellow
            }
            else {
                out.h = 4.0 + (this.r - this.g) / delta;  // between magenta & cyan
            }
        }
        out.h *= 60.0;                              // degrees

        while (out.h < 0.0) {
            out.h += 360.0;
        }
        while (out.h >= 360) {
            out.h -= 360.0;
        }

        return out;
    }

    set hsv(value) {
        if (!(value instanceof ColorHSV)) {
            throw Error("value must be ColorHSV");
        }
        const rgb = value.rgb;
        this.r = rgb.r;
        this.g = rgb.g;
        this.b = rgb.b;
    }

    get color() {
        return Phaser.Renderer.WebGL.Utils.getTintFromFloats(this.r, this.g, this.b, 1.0);
    }
}
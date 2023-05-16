#ifndef _H_COLOR_
#define _H_COLOR_

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

inline bool compare(const rgb& a, const rgb& b) {
    unsigned char ar = a.r * 255.0f;
    unsigned char ag = a.g * 255.0f;
    unsigned char ab = a.b * 255.0f;

    unsigned char br = b.r * 255.0f;
    unsigned char bg = b.g * 255.0f;
    unsigned char bb = b.b * 255.0f;

    return ar == br && ab == bb && ag == bg;
}

inline rgb lerp(const rgb& a, const rgb& b, const float t) {
    rgb result;
    result.r = a.r + (b.r - a.r) * t;
    result.g = a.g + (b.g - a.g) * t;
    result.b = a.b + (b.b - a.b) * t;
    if (result.r < 0.0f) {
        result.r = 0.0f;
    }
    if (result.g < 0.0f) {
        result.g = 0.0f;
    }
    if (result.r > 1.0f) {
        result.r = 1.0f;
    }
    if (result.g > 1.0f) {
        result.g = 1.0f;
    }
    if (result.b < 0.0f) {
        result.b = 0.0f;
    }
    if (result.b > 1.0f) {
        result.b = 1.0f;
    }
    return result;
}

inline hsv lerp(const hsv& a, const hsv& b, const float t) {
    hsv result;
    result.h = a.h + (b.h - a.h) * t;
    result.s = a.s + (b.s - a.s) * t;
    result.v = a.v + (b.v - a.v) * t;
    if (result.s < 0.0f) {
        result.s = 0.0f;
    }
    if (result.v < 0.0f) {
        result.v = 0.0f;
    }
    if (result.s > 1.0f) {
        result.s = 1.0f;
    }
    if (result.v > 1.0f) {
        result.v = 1.0f;
    }
    if (result.h < 0.0f) {
        result.h = 0.0f;
    }
    if (result.h > 360.0f) {
        result.h = 360.0f;
    }
    return result;
}

inline hsv rgb2hsv(const rgb& in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min < in.b ? min : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max > in.b ? max : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
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
        out.h = 0.0f;                            // its now undefined
        return out;
    }
    if (in.r >= max)                           // > is bogus, just keeps compilor happy
        out.h = (in.g - in.b) / delta;        // between yellow & magenta
    else
        if (in.g >= max)
            out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
        else
            out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if (out.h < 0.0)
        out.h += 360.0;

    return out;
}


inline rgb hsv2rgb(const hsv& in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch (i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

#endif
#include <stdio.h>

#include "firefly-color.h"

// Color Format:
//  - RGBA = 00AA AAAA  RRRR RRRR  GGGG GGGG  BBBB BBBB
//  - HSVA = N1AA AAAA  HHHH HHHH  HHHH SSSS  SSVV VVVV   (N => negative hue)
//  - Alpha is 0 for fully opaque and 0x20 for fully transparent


#define COLOR_HSV          (0x40000000)
#define NEGATIVE_HUE       (0x80000000)
#define MASK_ALPHA         (0x3f000000)

// OLD
////  - RGBA = 00AA AAAA  RRRR RRRR  GGGG GGGG  BBBB BBBB
////  - HSVA = 1NAA AAAA  HHHH HHHH  HHHH SSSS  SSVV VVVV   (N => negative hue)
//#define HSV_HUE(c)           ((((c) >> 12) & 0xfff) *(_IS_HUE_NEGATIVE(c) ? -1: 1))
//#define HSV_SAT(c)           (((c) >> 6) & 0x3f)
//#define HSV_VAL(c)           ((c) & 0x3f)
//#define ALPHA(c)             (0x3f - (((c) >> 24) & 0x3f))
//#define CLAMP(v,m)     (((v) < 0) ? 0: ((v) > (m)) ? (m): (v))
// Extract color components
//#define _IS_HUE_NEGATIVE(c)  (!!((c) & 0x40000000))
//#define IS_HSV(c)            (!!((c) & 0x80000000))
//  - Idea: Alpha is 0 for fully opaque and 0x20 for fully transparent


static uint32_t _getR(color_ffxt c) { return (c >> 16) & 0xff; }
static uint32_t _getG(color_ffxt c) { return (c >> 8) & 0xff; }
static uint32_t _getB(color_ffxt c) { return (c >> 0) & 0xff; }

//static bool _isHSV(color_t c) { return c & COLOR_HSV; }

static int32_t _getH(color_ffxt c) {
    int32_t hue = (c >> 12) & 0xfff;
    int32_t neg = 2 - (c >> 30);
    return neg * hue;
}

static uint32_t _getS(color_ffxt c) { return (c >> 6) & 0x3f; }
static uint32_t _getV(color_ffxt c) { return (c >> 0) & 0x3f; }

static uint32_t _getA(color_ffxt c) { return 0x20 - ((c >> 24) & 0x3f); }

#define CLAMP(v,m)     (((v) <= (m)) ? (v): (m))

color_ffxt ffx_color_rgb(int32_t r, int32_t g, int32_t b, int32_t alpha) {
    color_ffxt result = 0;

    result |= CLAMP(r, 0xff) << 16;
    result |= CLAMP(g, 0xff) << 8;
    result |= CLAMP(b, 0xff);
    result |= (0x20 - CLAMP(alpha, 0x20)) << 24;

    return result;
}

color_ffxt ffx_color_hsv(int32_t h, int32_t s, int32_t v, int32_t alpha) {
    color_ffxt result = COLOR_HSV;

    if (h < 0) {
        result |= NEGATIVE_HUE;
        h *= -1;
    }

    result |= (h % 3960) << 12;
    result |= CLAMP(s, 0x3f) << 6;
    result |= CLAMP(v, 0x3f);
    result |= (0x20 - CLAMP(alpha, 0x20)) << 24;

    return result;
}

// Stores RGB565 as a uint16_t
//#define RGB16(r,g,b)     ((((uint16_t)(r) & 0xf8) << 8) | (((uint16_t)(g) & 0xfc) << 3) | (((uint16_t)(b) & 0xf8) >> 3))

// Stores RGB888 as a uint32_t
//#define RGB24(r,g,b)    ((rgb24_t)(((r) << 16) | ((g) << 8) | (b)))

#define B6     (0x3f)

// Heavily inspired by: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
static color_ffxt _fromHSV(color_ffxt color) {

    int32_t h = _getH(color);// ((color >> 12) & 0xfff);
    int32_t s = _getS(color); //(color >> 6) & B6;
    int32_t v = _getV(color); //color & B6;
    //if (color & NEGATIVE_HUE) { h *= -1; }

    // Normalize h to [0, 359]; max h is 12 bits, so scale well past that
    h = (h + 360000) % 360;

    int32_t region = h / 60;
    int32_t rem = (h - (region * 60));

    // Normalize value to a byte
    v = (v * 255) / B6;

    // Notes:
    //  - v, p, and q MUST all be [0, 255] for the RGB mapping
    //  - s is [0, 63]
    //  - rem is [0, 59]
    //  - v, s and rem represent values [0, 1] multiplied by their multiplier
    //    - so (63 - s) is the equivlent of (1.0 - s_)
    //    - (v * s) / (255 * 63) is equivalent of (v_ * s_)

    // Mask out the two HSV-specific bits; keep the alpha
    color_ffxt result = color & MASK_ALPHA;

    int32_t p = (v * (63 - s)) / 63, q;

    // Map the RGB value based on the region
    switch (region) {
        case 0:
            q = (v * ((63 * 59) - (s * (59 - rem)))) / (63 * 59);
            result |= (v << 16) | (q << 8) | p;
            break;
        case 1:
            q = (v * ((63 * 59) - (s * rem))) / (63 * 59);
            result |= (q << 16) | (v << 8) | p;
            break;
        case 2:
            q = (v * ((63 * 59) - (s * (59 - rem)))) / (63 * 59);
            result |= (p << 16) | (v << 8) | q;
            break;
        case 3:
            q = (v * ((63 * 59) - (s * rem))) / (63 * 59);
            result |= (p << 16) | (q << 8) | v;
            break;
        case 4:
            q = (v * ((63 * 59) - (s * (59 - rem)))) / (63 * 59);
            result |= (q << 16) | (p << 8) | v;
            break;
        case 5:
            q = (v * ((63 * 59) - (s * rem))) / (63 * 59);
            result |= (v << 16) | (p << 8) | q;
            break;
    }

    return result;
}

static uint32_t min(uint32_t a, uint32_t b) {
    return (a < b) ? a: b;
}

static uint32_t max(uint32_t a, uint32_t b) {
    return (a > b) ? a: b;
}

static color_ffxt _fromRGB(color_ffxt color) {
    int32_t r = _getR(color), g = _getG(color), b = _getB(color);
    uint32_t rgbMin = min(r, min(g, b));
    uint32_t rgbMax = max(r, max(g, b));

    color_ffxt result = COLOR_HSV | (color & MASK_ALPHA);

    int32_t v = rgbMax;
    if (v == 0) { return result; }

    int32_t rgbDelta = rgbMax - rgbMin;

    int32_t s = 255 * rgbDelta / v;
    if (s == 0) { return result; }

    result |= (s & 0xfc) << 4;
    result |= (v >> 2);;

    if (rgbMax == r) {
        result |= (0 + 60 * (g - b) / rgbDelta) << 12;
    } else if (rgbMax == g) {
        result |= (120 + 60 * (g - b) / rgbDelta) << 12;
    } else {
        result |= (240 + 60 * (g - b) / rgbDelta) << 12;
    }

    return result;
}

rgb16_ffxt ffx_color_rgb16(color_ffxt color) {
    if (color & COLOR_HSV) { color = _fromHSV(color); }
    return RGB16((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

/*
rgba16_t ffx_color_rgba16(color_t color) {
    if (color & COLOR_HSV) { color = _fromHSV(color); }
    return RGB16((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff) |
      (ALPHA(color) << 16);
}
*/
rgb24_ffxt ffx_color_rgb24(color_ffxt color) {
    if (color & COLOR_HSV) { color = _fromHSV(color); }
    return color & 0x00ffffff;
}

rgba24_ffxt ffx_color_rgba24(color_ffxt color) {
    if (color & COLOR_HSV) { color = _fromHSV(color); }
    uint32_t alpha = (_getA(color) * 255) >> 4;
    return (alpha << 24) | (color & 0x00ffffff);
}

static int32_t lerp(int32_t a, int32_t b, fixed_ffxt t) {
    return scalarfx(a, FM_1 - t) + scalarfx(b, t);
}

color_ffxt ffx_color_lerpfx(color_ffxt c0, color_ffxt c1, fixed_ffxt t) {
    int32_t a = lerp(_getA(c0), _getA(c1), t);

    if (((c0 >> 31) + (c1 >> 31)) != 2) {
    }

    int32_t h = lerp(_getH(c0), _getH(c1), t);
    int32_t s = lerp(_getS(c0), _getS(c1), t);
    int32_t v = lerp(_getV(c0), _getV(c1), t);

    return ffx_color_hsv(h, s, v, a);
}

static int32_t lerpRatio(int32_t a, int32_t b, int32_t top, int32_t bottom) {
    return ((top * b) / bottom) + (((bottom - top) * a) / bottom);
}

color_ffxt ffx_color_lerpRatio(color_ffxt c0, color_ffxt c1,
  int32_t top, int32_t bottom) {

    int32_t a = lerpRatio(_getA(c0), _getA(c1), top, bottom);

    if ((c0 & COLOR_HSV) && (c1 & COLOR_HSV)) {
        int32_t h = lerpRatio(_getH(c0), _getH(c1), top, bottom);
        int32_t s = lerpRatio(_getS(c0), _getS(c1), top, bottom);
        int32_t v = lerpRatio(_getV(c0), _getV(c1), top, bottom);

        return ffx_color_hsv(h, s, v, a);
    }

    c0 = ffx_color_rgb24(c0);
    c1 = ffx_color_rgb24(c1);

    int32_t r = lerpRatio(_getR(c0), _getR(c1), top, bottom);
    int32_t g = lerpRatio(_getG(c0), _getG(c1), top, bottom);
    int32_t b = lerpRatio(_getB(c0), _getB(c1), top, bottom);

    return ffx_color_rgb(r, g, b, a);
}

/*
rgba16_t ffx_color_blend16(color_t fg, color_t bg) {
    uint32_t fga = ALPHA(fg);
    if (fga == 0) { return ffx_color_rgb16(bg); }
    if (fga == MAX_ALPHA) { return ffx_color_rgb(fg);
    return ffx_color_lerpfx(bg, fg, tofx(fga) / MAX_ALPHA);
}
*/

color_ffxt ffx_color_rgb2hsv(color_ffxt color) {
    if (color & COLOR_HSV) { return _fromRGB(color); }
    return color;
}

color_ffxt ffx_color_hsv2rgb(color_ffxt color) {
    if (color & COLOR_HSV) { return color; }
    return _fromHSV(color);
}

size_t ffx_color_name(color_ffxt c, char *name, size_t length) {
    if (c & COLOR_HSV) {
        return snprintf(name, length, "HSV(%ld, %ld/63, %ld/63, %ld/32)",
          _getS(c), _getS(c), _getV(c), _getA(c));
    }

    return snprintf(name, length, "RGBA(%ld/255, %ld/255, %ld/255, %ld/32)",
      _getR(c), _getG(c), _getB(c), _getA(c));
}


// Test program to validate HSV conversion
/*
int main() {
    for (uint32_t h  = 0; h < 360; h++) {
        for (uint32_t s  = 0; s <= 0x3f; s++) {
            for (uint32_t v = 0; v <= 0x3f; v++) {
                color_t hsv = color_hsv(h, s, v, 0xff);
                color_t rgb = _fromHSV(hsv);
                printf("h=%d, s=%d, v=%d, hsv=%u, rgb=%u\n", h,s, v, hsv, rgb);
            }
        }
    }
}
*/

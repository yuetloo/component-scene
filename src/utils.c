// typedef struct Bounds {
//     int16_t ix, iy;
//     uint8_t ox, oy, w, h;
// } Bounds;

typedef struct FfxClip {
    uint8_t srcX, srcY;
    uint8_t dstX, dstY;
    uint8_t width, height;
} FfxClip;

/**
 *  Given an input box (x, y, w, h) and a fragment (y0, height),
 *  update the input box values to reflect the box that is
 *  visible within the fragment (x, y, w, h).
 *
 *  Returns 1 if no part of the box overlaps the fragment.
 *
 *  int32_t ix = pos.x, iy = pos.y, w = size.width, h = size.height, ox, oy;
 *  if (calcBounds(&ix, &iy, &0x, &oy, &w, &h, y0, height)) { return; }
 }
 */
FfxClip ffx_scene_clip(FfxPoint pos, FfxSize size, int32_t y0, int32_t height) {
    FfxClip result = { .width = 0 };

    int32_t dstY = pos.y - y0;

    // The node is entirely below the the viewport; abort
    if (dstY >= height) { return result; }

    // The node is entirely to the right of the viewport; abort
    if (pox.x >= 240) { return result; }

    int32_t srcY = 0;

    // The input box is above the fragment, trim off some of the top
    int32_t adjustHeight = height;
    if (dstY < 0) {
      srcY -= offY;
      adjustHeight += offY;
      dstY = 0;
    }

    // The node is entirely above the viewport; abort
    if (adjustHeight <= 0) { return result; }

    int32_t srcX = 0;
    int32_t dstX = pos.x;
    if (dstX < 0) {
        srcX -= dstX;
        width += dstX;
        dstX = 0;
    }

    // The node is entirely to the left of the viewport; abort
    if (width <= 0) { return result; }

    // Extends past the fragment bounds; shrink
    if (dstY + adjustHeight > height) { adjustHeight = height - dstY; }
    if (dstX + width > 240) { width = 240 - dstX; }

    result.srcX = srcX;
    result.srcY = srcY;

    result.dstX = dstX;
    result.dstY = dstY;

    result.width = width;
    result.height = adjustHeight;

    return result;
}
/*
uint32_t calcBounds(int32_t *_pix, int32_t *_piy, int32_t *_ox, int32_t *_oy, int32_t *_w, int32_t *_h, int32_t y0, int32_t height) {
    // The top edge is belog the fragment; skip
    int32_t py = *_piy;
    if (py >= y0 + height) { return 1; }

    // The left edge is to the right of the fragment; skip
    int32_t px = *_pix;
    if (px >= 240) { return 1; }

    // Compute start y and height within the fragment
    int32_t iy = 0;
    int32_t oy = py - y0;
    int32_t h = *_h;
    if (oy < 0) {
        // The input box is above the fragment, trim off some of the top
        iy -= oy;
        h += oy;
        oy = 0;
    }
    if (h <= 0) { return 1; }

    // Compute the start x and width within the fragment
    int32_t ix = 0;
    int32_t ox = px;
    int32_t w = *_w;
    if (ox < 0) {
        // The input box is to the left of the fragment, trim off some of the left box
        ix -= ox;
        w += ox;
        ox = 0;
    }
    if (w <= 0) { return 1; }

    // Extends past the fragment bounds; shrink
    if (oy + h > height) { h = height - oy; }
    if (ox + w > 240) { w = 240 - ox; }

    // Copy the results back to the caller
    (*_pix) = ix;
    (*_piy) = iy;
    (*_ox) = ox;
    (*_oy) = oy;
    (*_w) = w;
    (*_h) = h;

    return 0;
}
*/

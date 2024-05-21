/**
 *  Scene Node: Box
 *
 *  a:Size     - (width, height)
 *  b:uint32   - color (16 bpp) repeated in top and bottom half
 */


#include "firefly-scene-private.h"


static void _boxRender(FfxPoint pos, FfxProperty a, FfxProperty b, uint16_t *frameBuffer, int32_t y0, int32_t height) {

    // The box is above the fragment or to the right of the display; skip
    if (pos.y >= y0 + height || pos.x >= 240) { return; }

    // Compute start y and height within the fragment
    int32_t sy = pos.y - y0;
    int32_t h = a.size.height;
    if (sy < 0) {
        h += sy;
        sy = 0;
    }
    if (h <= 0) { return; }

    // Compute the start x and width within the fragment
    int32_t sx = pos.x;
    int32_t w = a.size.width;
    if (sx < 0) {
        w += sx;
        sx = 0;
    }
    if (w <= 0) { return; }

    // Extends past the fragment bounds; shrink
    if (sy + h > height) { h = height - sy; }
    if (sx + w > 240) { w = 240 - sx; }

    uint16_t color = b.u32;
    for (uint32_t y = 0; y < h; y++) {
        uint16_t *output = &frameBuffer[240 * (sy + y) + sx];
        for (uint32_t x = 0; x < w; x++) {
//            uint16_t current = *output;
//            current = (current << 8) | (current >> 8);
//            current = ((current & 0xf000) >> 1) | ((current & 0x07c0) >> 1) | ((current & 0x001e) >> 1);
//            current = (current << 8) | (current >> 8);
//            *output++ = current;
              *output++ = color;
        }
    }
}

static void _boxSequence(FfxScene scene, FfxPoint worldPos, FfxNode node) {
    ffx_scene_createRenderNode(scene, node, worldPos, _boxRender);
}

FfxNode ffx_scene_createBox(FfxScene scene, FfxSize size, rgb16_t color) {
    FfxProperty a, b;
    a.size = size;
    b.u32 = __builtin_bswap16(color);
    return ffx_scene_createNode(scene, _boxSequence, a, b);
}

void scene_boxSetColor(FfxNode node, rgb16_t color) {
    FfxProperty *b = ffx_scene_nodePropertyB(node);
    b->u32 = __builtin_bswap16(color);
}

rgb16_t scene_boxColor(FfxNode node) {
    FfxProperty *b = ffx_scene_nodePropertyB(node);
    return __builtin_bswap16(b->u32);
}

void scene_boxSetSize(FfxNode node, FfxSize size) {
    FfxProperty *a = ffx_scene_nodePropertyA(node);
    a->size = size;
}

FfxSize scene_boxGetSize(FfxNode node) {
    FfxProperty *a = ffx_scene_nodePropertyA(node);
    return a->size;
}

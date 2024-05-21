/**
 *  Scene Node: Fill
 *
 *  a:uint32   - color (16 bpp) repeated in top and bottom half
 */


#include <stddef.h>

#include "firefly-scene-private.h"


static void _fillRender(FfxPoint pos, FfxProperty a, FfxProperty b, uint16_t *_frameBuffer, int32_t y0, int32_t height) {
    uint32_t *frameBuffer = (uint32_t*)_frameBuffer;

    uint32_t value = a.u32;

    for (uint32_t i = 0; i < 240 * height / 2; i++) { *frameBuffer++ = value; }
}

static void _fillSequence(FfxScene scene, FfxPoint worldPos, FfxNode node) {
    ffx_scene_createRenderNode(scene, node, PointZero, _fillRender);
}

FfxNode ffx_scene_createFill(FfxScene scene, rgb16_t color) {
    FfxProperty a, b;
    uint32_t c = __builtin_bswap16(color);
    a.u32 = ((c << 16) | c);
    b.ptr = NULL;

    return ffx_scene_createNode(scene, _fillSequence, a, b);
}

void ffx_scene_fillSetColor(FfxNode node, rgb16_t color) {
    FfxProperty *a = ffx_scene_nodePropertyA(node);

    uint32_t c = __builtin_bswap16(color);
    a->u32 = ((c << 16) | c);
}

rgb16_t scene_fillColor(FfxNode node) {
    if (node == NULL) { return 0; }
    FfxProperty *a = ffx_scene_nodePropertyA(node);
    return __builtin_bswap16(a->u32 & 0xffff);
}

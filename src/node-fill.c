/**
 *  Scene Node: Fill
 *
 *  a:uint32   - color (16 bpp) repeated in top and bottom half
 */


#include <stddef.h>
#include <stdio.h>

#include "firefly-scene-private.h"


static void _render(FfxPoint pos, FfxProperty a, FfxProperty b, uint16_t *_frameBuffer, int32_t y0, int32_t height) {
    uint32_t c = __builtin_bswap16(ffx_color_rgb16(a.color));
    c = ((c << 16) | c);

    uint32_t *frameBuffer = (uint32_t*)_frameBuffer;

    for (uint32_t i = 0; i < 240 * height / 2; i++) { *frameBuffer++ = c; }
}

static void _sequence(FfxScene scene, FfxPoint worldPos, FfxNode node) {
    ffx_scene_createRenderNode(scene, node, PointZero, _render);
}

static FfxNode _debug(FfxNode node, FfxNodeFunc func, char *descr,
  size_t length) {

    if (func.sequenceFunc == _sequence) {
        FfxProperty *a = ffx_scene_nodePropertyA(node);

        char color[32];
        ffx_color_name(a->color, color, sizeof(color));

        snprintf(descr, length,
          "Fill Sequence Node: color=%s", color);

    } else if (func.renderFunc == _render) {
        FfxProperty *a = ffx_scene_nodePropertyA(node);

        char color[32];
        ffx_color_name(a->color, color, sizeof(color));

        snprintf(descr, length,
          "Fill Render Node: color=%s", color);
    }

    return NULL;
}

FfxNode ffx_scene_createFill(FfxScene scene, color_ffxt color) {
    REGISTER_DEBUG(_debug);

    FfxProperty a;
    a.color = color;

    return ffx_scene_createNode(scene, _sequence, a, a);
}

color_ffxt* scene_fillColor(FfxNode node) {
    FfxProperty *a = ffx_scene_nodePropertyA(node);
    return &a->color;
}

/**
 *  Scene Node: Box
 *
 *  a:Size     - (width, height)
 *  b:uint32   - color (16 bpp) repeated in top and bottom half
 */

#include <stddef.h>
#include <stdio.h>

#include "firefly-scene-private.h"

static void _renderDark50(FfxPoint pos, FfxProperty a, FfxProperty b, uint16_t *frameBuffer, int32_t y0, int32_t height) {

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

//    rgb16_ffxt _color = ffx_color_rgb16(b.color);
//    uint16_t color = __builtin_bswap16(_color & 0xffff);

    //uint16_t color = b.u32;
    for (uint32_t y = 0; y < h; y++) {
        uint16_t *output = &frameBuffer[240 * (sy + y) + sx];
        for (uint32_t x = 0; x < w; x++) {
            uint16_t current = *output;
            current = (current << 8) | (current >> 8);
            current = ((current & 0xf000) >> 1) | ((current & 0x07c0) >> 1) | ((current & 0x001e) >> 1);
            current = (current << 8) | (current >> 8);
            *output++ = current;
        }
    }
}

static void _renderDark75(FfxPoint pos, FfxProperty a, FfxProperty b, uint16_t *frameBuffer, int32_t y0, int32_t height) {

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

//    rgb16_ffxt _color = ffx_color_rgb16(b.color);
//    uint16_t color = __builtin_bswap16(_color & 0xffff);

    //uint16_t color = b.u32;
    for (uint32_t y = 0; y < h; y++) {
        uint16_t *output = &frameBuffer[240 * (sy + y) + sx];
        for (uint32_t x = 0; x < w; x++) {
            uint16_t current = *output;
            current = (current << 8) | (current >> 8);
            current = ((current & 0xe000) >> 2) | ((current & 0x0780) >> 2) | ((current & 0x001c) >> 2);
            current = (current << 8) | (current >> 8);
            *output++ = current;
        }
    }
}

static void _render(FfxPoint pos, FfxProperty a, FfxProperty b, uint16_t *frameBuffer, int32_t y0, int32_t height) {

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

    rgb16_ffxt _color = ffx_color_rgb16(b.color);
    uint16_t color = __builtin_bswap16(_color & 0xffff);

    //uint16_t color = b.u32;
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

static void _sequence(FfxScene scene, FfxPoint worldPos, FfxNode node) {
    color_ffxt *color = ffx_scene_boxColor(node);
    if (*color == RGB_DARK50) {
        ffx_scene_createRenderNode(scene, node, worldPos, _renderDark50);
    } else if (*color == RGB_DARK75) {
        ffx_scene_createRenderNode(scene, node, worldPos, _renderDark75);
    } else {
        ffx_scene_createRenderNode(scene, node, worldPos, _render);
    }
}

static FfxNode _debug(FfxNode node, FfxNodeFunc func, char *descr,
  size_t length) {

    if (func.sequenceFunc == _sequence) {
        FfxPoint *pos = ffx_scene_nodePosition(node);
        FfxProperty *a = ffx_scene_nodePropertyA(node);
        FfxProperty *b = ffx_scene_nodePropertyB(node);

        char color[32];
        ffx_color_name(b->color, color, sizeof(color));

        snprintf(descr, length,
          "Box Sequence Node pos=(%d, %d), width=%d, height=%d, color=%s",
          pos->x, pos->y, a->size.width, a->size.height, color);

    } else if (func.renderFunc == _render) {
        FfxPoint *pos = ffx_scene_nodePosition(node);
        FfxProperty *a = ffx_scene_nodePropertyA(node);
        FfxProperty *b = ffx_scene_nodePropertyB(node);

        char color[32];
        ffx_color_name(b->color, color, sizeof(color));

        snprintf(descr, length,
          "Box Render Node pos=(%d, %d), width=%d, height=%d, color=%s",
          pos->x, pos->y, a->size.width, a->size.height, color);
    }

    return NULL;
}

FfxNode ffx_scene_createBox(FfxScene scene, FfxSize size, color_ffxt color) {
    REGISTER_DEBUG(_debug);

    FfxProperty a, b;
    a.size = size;
    b.color = color;
    return ffx_scene_createNode(scene, _sequence, a, b);
}

color_ffxt* ffx_scene_boxColor(FfxNode node) {
    FfxProperty *b = ffx_scene_nodePropertyB(node);
    return &b->color;
}
/*
void scene_boxSetSize(FfxNode node, FfxSize size) {
    FfxProperty *a = ffx_scene_nodePropertyA(node);
    a->size = size;
}
*/

FfxSize* scene_boxSize(FfxNode node) {
    FfxProperty *a = ffx_scene_nodePropertyA(node);
    return &a->size;
}

static void _animateColor(FfxNode node, fixed_ffxt t, FfxProperty p0, FfxProperty p1) {
    color_ffxt *color = ffx_scene_boxColor(node);
    *color = ffx_color_lerpfx(p0.color, p1.color, t);
}

uint32_t ffx_scene_boxAnimateColor(FfxScene scene, FfxNode node,
  color_ffxt target, uint32_t duration, FfxCurveFunc curve,
  FfxSceneAnimationCompletion onComplete) {

    FfxProperty *b = ffx_scene_nodePropertyB(node);

    FfxProperty start, end;
    start.color = b->color;
    end.color = target;

    FfxNode animate = ffx_scene_createAnimationNode(scene, node, start, end,
      duration, _animateColor, curve, onComplete);
    if (animate == NULL) { return 0; }

    return 1;
}

/*
int32_t debugDumpBox(FfxNode node, int32_t indent) {
    if (node->func.renderFunc == _boxRender) {
        printf("%s    - Box Render Node %ld (x=%d, y=%d, width=%d, height=%d, color=0x%04x)\n", padding,
            _getNodeIndex(scene, node), node->pos.x, node->pos.y, node->a.size.width, node->a.size.height,
            __builtin_bswap16 (node->b.u32));

if (node->func.sequenceFunc == _boxSequence) {
        printf("%s    - Box Node %ld (x=%d, y=%d, width=%d, height=%d, color=0x%04x)\n", padding,
            _getNodeIndex(scene, node), node->pos.x, node->pos.y, node->a.size.width, node->a.size.height,
            __builtin_bswap16(node->b.u32));
*/

/**
 *  Scene Node: Image
 *
 *  The image data formats supported include a header which indicates
 *  which specialized RenderFun should be used.
 *
 *  a:uint8_t* - a pointer the image data
 *  b:Size     - this value depends on the image type, and may
 *               represent multiple things, such as alpha or a
 *               mask value, pallette information or
 *               pre-computed data
 */


#include <stddef.h>

#include "firefly-scene-private.h"


static  void _imageRender(FfxPoint pos, FfxProperty a, FfxProperty b,
  uint16_t *frameBuffer, int32_t y0, int32_t height) {
    // printf("SS: %d %d", b.size.width, b.size.height);

    // @TODO: Move quick checks to _sequence?

    // The box is above the fragment or to the right of the display; skip
    if (pos.y >= y0 + height || pos.x >= 240) { return; }

    // Compute start y and height within the fragment
    int32_t iy = 0;
    int32_t oy = pos.y - y0;
    int32_t oh = b.size.height;
    if (oy < 0) {
        iy -= oy;
        oh += oy;
        oy = 0;
    }
    if (oh <= 0) { return; }

    // Compute the start x and width within the fragment
    int32_t ix = 0;
    int32_t ox = pos.x;
    const int32_t w = b.size.width;
    int32_t ow = w;
    if (ox < 0) {
        ix -= ox;
        ow += ox;
        ox = 0;
    }
    if (ow <= 0) { return; }

    // Extends past the fragment bounds; shrink
    if (oy + oh > height) { oh = height - oy; }
    if (ox + ow > 240) { ow = 240 - ox; }

    // Skip the header bytes
    ix += 2;

    uint16_t *data = (uint16_t*)(a.ptr);
    for (int32_t y = oh; y; y--) {
        uint16_t *output = &frameBuffer[(240 * (oy + y - 1)) + ox];
        uint16_t *input = &data[((iy + y - 1) * w) + ix];
        for (int32_t x = ow; x; x--) {
            *output++ = *input++;
        }
    }
}

static void _imageSequence(FfxScene scene, FfxPoint worldPos, FfxNode node) {
    ffx_scene_createRenderNode(scene, node, worldPos, _imageRender);
    // @TODO: Do simple checks here and include ix, ox, etc in the b?
}

static FfxSize _imageValidate(const uint16_t *data, uint32_t dataLength) {
    FfxSize size;
    size.width = (data[1] >> 8) + 1;
    size.height = (data[1] & 0xff) + 1;
    // @TODO validate image data

    return size;
}

FfxNode ffx_scene_createImage(FfxScene scene, const uint16_t *data,
  uint32_t dataLength) {
    //_Scene* scene = _scene;

    FfxSize size = _imageValidate(data, dataLength);
    if (size.width == 0 || size.height == 0) { return NULL; }

    FfxProperty a, b;
    a.ptr = (void*)data;
    b.size = size;

    return ffx_scene_createNode(scene, _imageSequence, a, b);

    //_Node* node = _addNode(scene, PointZero);
    //if (node == NULL) { return NULL; }

    //node->func.sequenceFunc = _imageSequence;

    //node->a.ptr = data;
    //node->b.size = size;

    //return node;
}

uint16_t* ffx_scene_imageData(FfxNode node) {
    FfxProperty *a = ffx_scene_nodePropertyA(node);
    return a->ptr;
}

FfxSize ffx_scene_imageSize(FfxNode node) {
    if (node == NULL) { return SizeZero; }

    FfxProperty *b = ffx_scene_nodePropertyB(node);

    return b->size;
}

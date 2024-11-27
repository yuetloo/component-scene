/**
 *  Scene Node: Text
 *
 *  A text label.
 *
 *  a:uint8_t* - a pointer the text data (type specific)
 *  b:TextInfo - info for the text
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "firefly-scene-private.h"
#include "firefly-color.h"

#include "font.h"


#define TextFlagModeConst      (0x04)
#define TextFlagModeAlloc      (0x02)

#define FONT_BASELINE     ((32 - 6) + 1)
#define FONT_HEIGHT       (32)
#define FONT_WIDTH        (16)

/*
uint32_t fixedMul(uint32_t a, uint32_t b) {
    uint64_t result = ((uint64_t)a) * ((uint64_t)b);
    return result >> 32;
}

uint32_t fixedDiv(uint32_t num, uint32_t den) {
    uint64_t result = (((uint64_t)num) << ((uint64_t)32)) / ((uint64_t)den);
    return result;
}
*/


// typedef struct Bounds {
//     int16_t ix, iy;
//     uint8_t ox, oy, w, h;
// } Bounds;
/**
 *  Given an input box (x, y, w, h) and a fragment (y0, height), update the input box values to reflect
 *  the box that is visible within the fragment (x, y, w, h). Returns 1 if no part of the box overlaps
 *  the fragment.
 *
 *  int32_t ix = pos.x, iy = pos.y, w = size.width, h = size.height, ox, oy;
 *  if (calcBounds(&ix, &iy, &0x, &oy, &w, &h, y0, height)) { return; }
 }
 */
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


static  void _textRender(FfxPoint pos, FfxProperty a, FfxProperty b, uint16_t *frameBuffer, int32_t y0, int32_t height) {
    pos.y -= FONT_BASELINE;

    // The text is above the fragment or to the right of the display; skip
    if (pos.y >= y0 + height || pos.x >= 240 || pos.y + FONT_HEIGHT < y0) { return; }

    FfxTextInfo info = b.text;

    int32_t length = info.length;
    int32_t textWidth = ((FONT_WIDTH + 1) * length) - 1;

    // The *max* text length is to the left of the screen
    if (pos.x + textWidth < 0) { return; }

    // Get the actual text content; for non-constant text, select the correct page
    uint8_t *text = a.data;
//    if ((info.flags & TextFlagModeConst) == 0) { text = &text[length]; }

    // Recompute the length and width for the *actual* text
    length = strnlen((char*)text, length);
    if (length == 0) { return; }
    textWidth = ((FONT_WIDTH + 1) * length) - 1;

    // The text length is to the left of the screen
    if (pos.x + textWidth < 0) { return; }

    // Compute start input-y, output-y and output-height, clipping to fragment bounds
    int32_t iy = 0;
    int32_t oy = pos.y - y0;
    int32_t oh = FONT_HEIGHT;
    if (oy < 0) {
        iy -= oy;
        oh += oy;
        oy = 0;
    }
    if (oh <= 0) { return; }

    // Compute the start input-x, output-x and output-width, clipping to fragment bounds
    int32_t ix = 0;
    int32_t ox = pos.x;
    const int32_t w = textWidth;
    int32_t ow = w;
    if (ox < 0) {
        ix -= ox;
        ow += ox;
        ox = 0;
    }
    if (ow <= 0) { return; }

    // Extends past the fragment bounds; clip to fragment bounds
    if (oy + oh > height) { oh = height - oy; }
    if (ox + ow > 240) { ow = 240 - ox; }

    // Fast-forward to the first on-screen character and track how many bits to trim from the left
    int32_t si = ix / (FONT_WIDTH + 1);
    text = &text[si];
    length -= si;
    si = ix - si * (FONT_WIDTH + 1);

    // For each on-screen (including partial) character...
    for (int32_t i = 0; i < length; i++) {
        // Past the right edge of the screen; done
        if (ox >= 240) { break; }

        uint8_t c = text[i];

        // Null-termination; done
        if (c == 0) { break; }

        // Space; just advance the cursor
        if (c == 32) {
            ox += FONT_WIDTH + 1 - si;
            si = 0;
            continue;
        }

        // Normalize the code to our font glyph indices
        c -= 33;
        if (c >= 94) { c = 0; } // @TODO: unsupported char

        // Get the font metrics and data offset
        int32_t top = FontData[2 * c + 1] >> 8;
        if (oy + top >= height) {
            ox += FONT_WIDTH + 1 - si;
            si = 0;
            continue;
        }

        uint32_t offset = FontData[2 * c];
        int32_t rows = FontData[2 * c + 1] & 0xff;

        // Adjust the character-output-y and character-output-height (and possibly font metric rows)
        int32_t coy = oy;
        int32_t coh = oh;
        {
            int32_t dy = iy - top;
            if (dy < 0) {
                coy -= dy;
                coh += dy;
            } else {
                offset += dy;
                rows -= dy;
            }
            if (coh > rows) { coh = rows; }
        }

        const uint16_t *fontData = &FontData[offset];

        // Draw the character rows
        for (int32_t y = 0; y < coh; y++) {
            // Past the bottom edge of the screen; done this character
            if (coy + y >= height) { break; }

            // Target in the frame buffer fragment to blit to
            uint16_t *output = &frameBuffer[(240 * (coy + y)) + ox];

            // Get the font row data, adjusting for trimming off-screen pixels
            uint32_t row = *fontData++;
            {
                int32_t ex = 240 - ox;
                if (ex <= FONT_WIDTH) { row &= (1 << ex) - 1; }
            }
            row >>= si;

            // Blit the pixel data to the frame buffer fragment
            while(row) {
                if (row & 0x01) {
                    *output++ = 0xffff;
                } else {
                    (void)(*output++);
                }
                row >>= 1;
            }
        }

        // Advance the cursor
        ox += FONT_WIDTH + 1 - si;
        si = 0;
    }
}

static void _textSequence(FfxScene scene, FfxPoint worldPos, FfxNode node) {
    FfxNode render = ffx_scene_createRenderNode(scene, node, worldPos, _textRender);
    if (render == NULL) { return; }

    FfxProperty *b = ffx_scene_nodePropertyB(node);

    // @TODO: Double check this access; pointers to unions
    FfxTextInfo *info = &b->text;
    if (!(info->flags & TextFlagModeConst)) {
        FfxProperty *a = ffx_scene_nodePropertyA(node);

        char *text = (char*)(a->data);

        // Copy the set value to the render value
        memcpy(&text[0], &text[info->length], info->length);
    }
}

FfxNode ffx_scene_createText(FfxScene scene, const char* data, uint32_t dataLength) {
    FfxProperty a, b;
    a.data = (uint8_t*)data;
    FfxTextInfo info = { .flags = TextFlagModeConst, .length = 255, .color = 0x0000 };
    b.text = info;

    return ffx_scene_createNode(scene, _textSequence, a, b);
}

FfxNode ffx_scene_createTextStr(FfxScene scene, const char* text) {
    return ffx_scene_createText(scene, text, strlen(text));
}

FfxNode ffx_scene_createTextFlip(FfxScene scene, char* data, uint32_t dataLength) {
    FfxProperty a, b;
    a.data = (uint8_t*)data;
    FfxTextInfo info = { .flags = 0, .length = dataLength / 2, .color = 0x0000 };
    b.text = info;

    return ffx_scene_createNode(scene, _textSequence, a, b);
}

FfxNode ffx_scene_createTextAlloc(FfxScene scene, uint32_t textLength) {
    uint8_t *data = malloc(textLength * 2);
    if (data == NULL) { return NULL; }

    FfxProperty a, b;
    a.data = data;
    FfxTextInfo info = { .flags = TextFlagModeAlloc, .length = textLength, .color = 0x0000 };
    b.text = info;

    return ffx_scene_createNode(scene, _textSequence, a, b);
}

void ffx_scene_textSetText(FfxNode node, const char* const text, uint32_t _length) {
    FfxProperty *b = ffx_scene_nodePropertyB(node);

    FfxTextInfo *info = &b->text;

    // Cannot modify a const memory address
    if (info->flags & TextFlagModeConst) { return; }

    uint32_t length = b->text.length;
    if (_length < length) { length = _length; }

    FfxProperty *a = ffx_scene_nodePropertyA(node);

    strncpy((char*)(&a->data[info->length]), text, length);
}

size_t ffx_scene_textGetText(FfxNode node, char *text, size_t _length) {
    if (node == NULL) { return 0; }

    FfxProperty *a = ffx_scene_nodePropertyA(node);
    FfxProperty *b = ffx_scene_nodePropertyB(node);

    uint32_t length = b->text.length;
    if (_length < length) { length = _length; }

    strncpy(text, (char*)a->data, length);

    return length;
}

void ffx_scene_textSetTextInt(FfxNode node, int32_t value) {
    static char buffer[12];
    size_t length = snprintf(buffer, sizeof(buffer), "%ld", value);
    ffx_scene_textSetText(node, buffer, length);
}

void ffx_scene_textSetColor(FfxNode node, rgb16_ffxt color) {
    FfxProperty *a = ffx_scene_nodePropertyA(node);
    a->text.color = color;
}
/*
void scene_textSetColorAlpha(Node _node, rgba_t color) {
    _Node *node = _node;
    if (node == NULL) { return; }

    node->a.text.color = color & 0xffff;
    node->a.text.flags |= (color >> 16) & 0xf8;
}
*/

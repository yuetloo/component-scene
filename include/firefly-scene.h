#ifndef __FIREFLY_SCENE_H__
#define __FIREFLY_SCENE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#include "firefly-color.h"
#include "firefly-curves.h"


// Scene Context Object (opaque; do not inspect or rely on internals)
typedef void* FfxScene;

// Scene Node Object (opaque; do not inspect or rely on internals)
typedef void* FfxNode;

// Point
typedef struct FfxPoint {
    int16_t x;
    int16_t y;
} FfxPoint;

extern const FfxPoint PointZero;
//extern const FfxPoint PointHidden;

// Size
typedef struct FfxSize {
    uint16_t width;
    uint16_t height;
} FfxSize;

extern const FfxSize SizeZero;

// Animation

typedef enum FfxSceneActionStop {
    FfxSceneActionStopNormal      = 0,
    FfxSceneActionStopCurrent     = (1 << 1) | (0 << 0),
    FfxSceneActionStopFinal       = (1 << 1) | (1 << 0),
} FfxSceneActionStop;

/*
typedef enum SceneActionOption {
    SceneActionOptionNone      = 0,
    SceneActionOptionReverse   = (1 << 3),
    SceneActionOptionRepeat    = (1 << 4),
} SceneActionOption;
*/

/*
typedef enum SceneNodeEffect {
    SceneNodeEffectDarken     = 1,
    SceneNodeEffectLighten    = 2,
} SceneNodeEffect;
*/

typedef void (*FfxSceneAnimationCompletion)(FfxScene scene,
  FfxNode node, FfxSceneActionStop stopType);

// typedef enim SceneFilter {
//     SceneFilterDarken,
//     SceneFilterLighten,
// //    SceneFilterBlur
// } SceneFilter;


// Allocate a new SceneContext
FfxScene ffx_scene_init(uint32_t nodeCount);
//SceneContext scene_initStatic(uint8_t *data, uint32_t dataLength);

// Release a ScreenContext created using scene_init.
void ffx_scene_free(FfxScene scene);

// Debugging; dump the scene graph to the console
void ffx_scene_dump(FfxScene scene);

// Rendering
uint32_t ffx_scene_sequence(FfxScene scene);
void ffx_scene_render(FfxScene scene, uint8_t *fragment, int32_t y0,
  int32_t height);

// Get the root node of the scene
FfxNode ffx_scene_root(FfxScene scene);

// Returns true if node is running any animations
uint32_t ffx_scene_isRunningAnimation(FfxNode node);

// Stop all current animations on a node
void ffx_scene_stopAnimations(FfxNode node, FfxSceneActionStop stopType);

// Schedule the node to be freed on the next sequence
void ffx_scene_nodeFree(FfxNode node);

int32_t ffx_scene_nodeTag(FfxNode node);
int32_t ffx_scene_nodeSetTag(FfxNode node, int32_t tag);

// Move a node within the scene (with respect to its parents in the hierarchy)
//void ffx_scene_nodeSetPosition(FfxNode node, FfxPoint pos);
FfxPoint* ffx_scene_nodePosition(FfxNode node);

uint32_t ffx_scene_nodeAnimatePosition(FfxScene scene, FfxNode node,
    FfxPoint target, uint32_t duration, FfxCurveFunc curve,
    FfxSceneAnimationCompletion onComplete);

// Create a new GroupNode
FfxNode ffx_scene_createGroup(FfxScene scene);

// Add a child to the end of the children for a parent GroupNode (created using scene_createGroup)
void ffx_scene_appendChild(FfxNode parent, FfxNode child);

// Create a FillNode, filling the entire screen with color
FfxNode ffx_scene_createFill(FfxScene scene, uint16_t color);
void ffx_scene_fillSetColor(FfxNode node, rgb16_t color);
rgb16_t ffx_scene_fillColor(FfxNode node);

uint32_t ffx_scene_fillAnimateColor(FfxScene scene, FfxNode node,
    uint16_t targetColor, uint32_t duration, FfxCurveFunc curve,
    FfxSceneAnimationCompletion onComplete);

// Create a BoxNode with width and height filled with color.
FfxNode ffx_scene_createBox(FfxScene scene, FfxSize size, uint16_t color);
void ffx_scene_boxSetColor(FfxNode node, rgb16_t color);
//void scene_boxSetColorAlpha(Node node, rgba_t color);
void ffx_scene_boxSetSize(FfxNode node, FfxSize size);
rgb16_t ffx_scene_boxColor(FfxNode node);
FfxSize ffx_scene_boxGetSize(FfxNode node);

uint32_t ffx_scene_boxAnimateColor(FfxScene scene, FfxNode node,
    rgb16_t target, uint32_t duration, FfxCurveFunc curve,
    FfxSceneAnimationCompletion onComplete);
//uint32_t scene_boxAnimateColorAlpha(SceneContext scene, Node node,
//    rgba16_t target, uint32_t duration, CurveFunc curve, SceneAnimationCompletion onComplete);
uint32_t ffx_scene_boxAnimateSize(FfxScene scene, FfxNode node,
    FfxSize target, uint32_t duration, FfxCurveFunc curve,
    FfxSceneAnimationCompletion onComplete);

// Images
FfxNode ffx_scene_createImage(FfxScene scene, const uint16_t *data,
  uint32_t dataLength);
uint16_t* ffx_scene_imageData(FfxNode node);
FfxSize ffx_scene_imageSize(FfxNode node);

// A spritesheet is a 32x32 tile node where each tile is an index into the spriteData for that position
// - The spriteData is a 256 2-byte entries
// Node scene_createSpritesheet(SceneContext context, uint8_t *spriteData);
// void scene_spriteheetSetSprite(Node spritesheet, uint8_t ix, uint8_t iy, uint8_t index);
// uint8_t *scene_spritesheetGetIndices(Node spritesheet);

/********************************************************
 * TEXT NODE
 ********************************************************/

// Create a TextNode backed by static content, up to dataLength bytes long.
// Text can NOT be updated using scene_setText and must NOT change.
FfxNode ffx_scene_createText(FfxScene scene, const char* data,
  uint32_t dataLength);

// Create a TextNode backed by provided data, up to floor(dataLength / 2)
// long, which can be updated using scene_textSetText. The data is split
// into two fragments, which are swapped on sequencing. If there is data
// present, the first half will be rendered.
FfxNode ffx_scene_createTextFlip(FfxScene scene, char* data,
  uint32_t dataLength);

// Create a TextNode which will allocate the necessary memory for
// dataLength strings, which can be updated using scene_textSetText.
FfxNode ffx_scene_createTextAlloc(FfxScene scene, uint32_t textLength);

void ffx_scene_textSetText(FfxNode node, const char* const text,
  uint32_t length);
void ffx_scene_textSetTextInt(FfxNode node, int32_t value);
void ffx_scene_textSetColor(FfxNode node, rgb16_t color);
//void scene_textSetColorAlpha(Node node, rgba_t color);

uint32_t ffx_scene_textAnimateColor(FfxScene scene, FfxNode node,
    rgb16_t target, uint32_t duration, FfxCurveFunc curve,
    FfxSceneAnimationCompletion onComplete);
//uint32_t scene_textAnimateColorAlpha(SceneContext scene, Node node,
//    rgba_t target, uint32_t duration, CurveFunc curve, SceneAnimationCompletion onComplete);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FIREFLY_SCENE_H__ */

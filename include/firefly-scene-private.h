#ifndef __FIREFLY_SCENE_PRIVATE_H__
#define __FIREFLY_SCENE_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "firefly-scene.h"


/**
 *  These functions can be used to create custom Nodes for
 *  the scene graph.
 */

//////////////////////////////
// Custom nodes


typedef struct FfxTextInfo {
    // 5 bits alpha, 1 bit isConst, 1 bit isAlloc, 1 bit flipPage
    uint8_t flags;
    uint8_t length;
    rgb16_ffxt color;
} FfxTextInfo;

/**
 *  A generic property type to encapsulate many values into 4 bytes.
 */
typedef union FfxProperty {
    void* ptr;
    uint8_t* data;
    uint32_t u32;
    int32_t i32;
    FfxPoint point;
    FfxSize size;
    FfxTextInfo text;
    color_ffxt color;
    FfxSceneAnimationCompletion animationCompletion;
} FfxProperty;


// This function is called on sequence and should add the
// necessary (if any) render nodes using [[createRenderNode]].
typedef void (*FfxSequenceFunc)(FfxScene context, FfxPoint pos, FfxNode node);

// This function should render the viewport given by
// (y0, height) into the frameBuffer, with the node properties.
typedef void (*FfxRenderFunc)(FfxPoint pos, FfxProperty a, FfxProperty b,
   uint16_t *frameBuffer, int32_t y0, int32_t height);

typedef void (*FfxAnimateFunc)(FfxNode node, fixed_ffxt t, FfxProperty p0,
  FfxProperty p1);


FfxNode ffx_scene_createNode(FfxScene scene, FfxSequenceFunc sequenceFunc,
  FfxProperty a, FfxProperty b);

FfxNode ffx_scene_createRenderNode(FfxScene scene, FfxNode sceneNode,
  FfxPoint pos, FfxRenderFunc renderFunc);

FfxNode ffx_scene_createAnimationNode(FfxScene scene, FfxNode node,
  FfxProperty start, FfxProperty end, uint32_t duration,
  FfxAnimateFunc animateFunc, FfxCurveFunc curveFunc,
  FfxSceneAnimationCompletion onComplete);

// Return pointers to the given property of a %%node%% to
// be read or updated.
FfxProperty* ffx_scene_nodePropertyA(FfxNode node);
FfxProperty* ffx_scene_nodePropertyB(FfxNode node);

//////////////////////////////
// Debugging

typedef union FfxNodeFunc {
    FfxSequenceFunc sequenceFunc;
    FfxRenderFunc renderFunc;
    FfxAnimateFunc animateFunc;
    FfxCurveFunc curveFunc;
} FfxNodeFunc;

// Maximum FfxSceneDumpNodeFuncs to allow registration
// Note: Set to 0 to disable Node Dump debugging
#define FFX_SCENE_DEBUG    (16)

// Write a description of the %%node%% to %%descr%% at most %%length%%
// bytes long.
// The return value is any child, which will be processed with indent + 1
typedef FfxNode (*FfxSceneDumpNodeFunc)(FfxNode node, FfxNodeFunc func,
    char *descr, size_t length);

int32_t ffx_scene_registerDebug(FfxSceneDumpNodeFunc dumpNode);
void ffx_scene_dumpScene(FfxScene scene);

#define REGISTER_DEBUG(debug) {\
    static uint32_t reg = 0; \
    if (!reg) { reg = ffx_scene_registerDebug(_debug); } \
}


//////////////////////////////
// Utilities

typedef struct FfxClip {
    uint8_t srcX, srcY;
    uint8_t dstX, dstY;
    uint8_t width, height;
} FfxClip;

/**
 *  Given an input box (pos, size) and the viewport
 *  ((0, y0), (240, height)), compute the clipping region of
 *  the input box which overlaps the viewport and where it
 *  maps to within that viewport.
 *
 *  If node is entirely outside the viewport, then width is
 *  0 and the node can be skipped entirely.
 */
FfxClip ffx_scene_clip(FfxPoint pos, FfxSize size, int32_t y0, int32_t height);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FIREFLY_SCENE_PRIVATE_H__ */

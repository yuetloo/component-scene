#ifndef __FIREFLY_INTERNAL_SCENE_H__
#define __FIREFLY_INTERNAL_SCENE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "firefly-scene-private.h"

struct _Node;
struct _Scene;

typedef union NodeFunc {
    FfxSequenceFunc sequenceFunc;
    FfxRenderFunc renderFunc;
    FfxAnimateFunc animateFunc;
    FfxCurveFunc curveFunc;
} NodeFunc;

typedef union AnimateProp {
    struct _Node *node;
    FfxSceneAnimationCompletion onComplete;
} AnimateProp;

typedef struct _Node {
    struct _Node *nextNode;

    // User defined values
    int32_t tag;

    // Depends on the type of node:
    //  - Sequence    => List of [ Animate, PropAnimate ] nodes
    //  - PropAnimate => onComplete callback
    AnimateProp animate;

    NodeFunc func;
    FfxPoint pos;
    FfxProperty a;
    FfxProperty b;
} _Node;

typedef struct _Scene {
    // All nodes in the scene, including render nodes
    _Node *nodes;

    // The pointer to the next free node to allocate
    _Node *nextFree;

    // The root (group) node
    _Node *root;

    // Gloabl tick
    int32_t tick;

    //  The head and tail of the current render list (may be null)
    _Node *renderHead;
    _Node *renderTail;

} _Scene;

// This *actually* removes the node; it should not be used directly
// unless you are exceptionally sure you know what you are doing.
// USe this when a node has been allocated but not added to the scene.
void _freeNode(_Scene *scene, _Node *node);

void _freeSequence(FfxScene scene, FfxPoint worldPos, FfxNode node);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FIREFLY_INTERNAL_SCENE_H__ */

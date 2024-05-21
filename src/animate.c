#include <stddef.h>

#include "scene.h"

/*
// @TODO: return uint32?
FfxNode ffx_scene_createAnimationNode(FfxScene _scene, FfxNode _node, FfxProperty a, Property b, uint32_t duration, AnimateFunc animateFunc, CurveFunc curveFunc, FfxSceneAnimationCompletion onComplete) {
    _SceneContext *scene = _scene;
    _Node *node = _node;

    _Node* animate = _addNode(scene, PointZero);
    if (animate == NULL) { return NULL; }

    _Node *prop = _addNode(scene, PointZero);
    if (prop == NULL) {
        _freeNode(scene, animate);
        return NULL;
    }

    animate->nextNode = prop;
    animate->func.animateFunc = animateFunc;
    animate->a.i32 = duration;
    animate->b.i32 = scene->tick + duration;

    if (onComplete) {
        prop->animate.onComplete = onComplete;
    }
    prop->func.curveFunc = curveFunc;

    prop->nextNode = node->animate.node;;
    node->animate.node = animate;

    prop->a = a;
    prop->b = b;

    //return animate;
    return prop;
}
*/
/*
static void _updateAnimations(_Scene *scene, _Node *node) {

    _Node *animate = node->animate.node;

    // No animations
    if (animate == NULL) { return; }

    int32_t now = xTaskGetTickCount();
    scene->tick = now;

    _Node *lastAnimate = NULL;

    uint16_t stopType = SceneActionStopNormal;

    while (animate) {
        int32_t duration = animate->a.i32;
        int32_t endTime = animate->b.i32;

        _Node *prop = animate->nextNode;
        _Node *nextAnimate = prop->nextNode;

        if (stopType == SceneActionStopNormal) { stopType = animate->pos.y; }
        if (stopType) { endTime = now; }

        if (stopType != SceneActionStopCurrent) {
            // Compute the curve-adjusted t
            // fixed_t t = FM_1 - divfx((endTime - now) << 16, duration << 16);
            fixed_t t = FM_1 - (tofx(endTime - now) / duration);
            if (t >= FM_1) { t = FM_1; }

            t = prop->func.curveFunc(t);

            // Perform the animation
            animate->func.animateFunc(node, t, prop->a, prop->b);
        }

        // Remove animation
        if (now >= endTime) {

            // Make the previous AnimateNode link to the next
            if (lastAnimate) {
                lastAnimate->nextNode->nextNode = nextAnimate;
            } else {
                node->animate.node = nextAnimate;
            }

            // Add the node back to the free nodes
            prop->nextNode = scene->nextFree;
            scene->nextFree = animate;

            if (prop->animate.onComplete) {
                prop->animate.onComplete(scene, node, stopType);
            }

        } else {
            lastAnimate = animate;
        }

        // Advance to the next animation
        animate = nextAnimate;
    }
}
*/
uint32_t ffx_scene_isRunningAnimation(FfxNode _node) {
    _Node *node = (_Node*)_node;
    if (node == NULL) { return 0; }

    return (node->animate.node == NULL) ? 0: 1;
}

void ffx_scene_stopAnimations(FfxNode _node, FfxSceneActionStop stopType) {
    _Node *node = (_Node*)_node;
    if (node == NULL || node->animate.node == NULL) { return; }

    // Schedule the first animation to cancel it and the following animations
    node->animate.node->pos.y = stopType;
}


// @TODO: drop SceneContext from this?
/*
static void _nodeAnimatePositionHoriz(_Node *node, fixed_t t, Property p0, Property p1) {
    node->pos.x = p0.point.x + scalarfx(p1.point.x - p0.point.x, t);
}

static void _nodeAnimatePositionVert(_Node *node, fixed_t t, Property p0, Property p1) {
    node->pos.y = p0.point.y + scalarfx(p1.point.y - p0.point.y, t);
}

static void _nodeAnimatePosition(_Node *node, fixed_t t, Property p0, Property p1) {
    Point result = p0.point;
    result.x += scalarfx(p1.point.x - p0.point.x, t);
    result.y += scalarfx(p1.point.y - p0.point.y, t);
    node->pos = result;
}

uint32_t scene_nodeAnimatePosition(SceneContext _scene, Node _node, Point target, uint32_t duration, CurveFunc curve, SceneAnimationCompletion onComplete) {
    _SceneContext *scene = _scene;
    _Node *node = _node;

    AnimateFunc animateFunc = _nodeAnimatePosition;
    if (node->pos.x == target.x) {
        animateFunc = _nodeAnimatePositionVert;
    } else if (node->pos.y == target.y) {
        animateFunc = _nodeAnimatePositionHoriz;
    }

    _Node *animate = _addAnimationNode(scene, node, duration, animateFunc, curve, onComplete);
    if (animate == NULL) { return 0; }

    animate->nextNode->a.point = node->pos;
    animate->nextNode->b.point = target;

    return 1;
}

*/

/********************************************************
 * ANIMATION COMPLETE NODE
 ********************************************************/
/*
static void _animationCompletionSequence(_SceneContext *scene, Point worldPos, _Node* node) {
    uint32_t animationId = (node->pos.x << 16) | node->pos.y;

    // Still running; do nothing yet
    if (_getAnimationNode(scene, animationId)) { return; }

    // Prevent me from every running again and schedule me to be freed
    node->func.sequenceFunc = _freeSequence;
    node->pos.x = node->pos.y = 0;

    // Call the callback
    node->a.animationCompletion(scene, node->b.ptr, 0);
}
*/
/*
uint32_t scene_onAnimationCompletion(SceneContext _scene, AnimationId animationId, SceneAnimationCompletion callback, void *context) {
    _SceneContext *scene = _scene;

    // Animation has already completed
    _Node *animate = _getAnimationNode(scene, animationId);
    if (animate == NULL) { return 0; }

    _Node* node = _addNode(scene, PointZero);
    if (node == NULL) { return 0; }

    // Add the node to the scene to be sequenced with Scene Nodes (after the animation nodes have been sequenced)
    _appendChild(scene->root, node);

    node->pos.x = animationId >> 16;
    node->pos.y = animationId & 0x7fff;
    node->a.animationCompletion = callback;
    node->b.ptr = (context != NULL) ? context: animate->a.ptr;
    node->func.sequenceFunc = _animationCompletionSequence;

    return 1;
}
*/

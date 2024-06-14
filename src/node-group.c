/**
 *  Scene Node: Group
 *
 *  A Group Node does not have any visible objects itself, but
 *  allows for hierarchal scenes with multiple "attached" objects
 *  to be moved together.
 *
 *  x, y       - co-ordinates in local space
 *  a:void*    - a pointer to the first child
 *  b:void*    - a pointer to the last child
 */

// The group node is a bit special and requires lower-level access
// to the Scene and Nodes than normal nodes, so we access scene.h
// instead of firefly-scene-private.h

#include <stddef.h>
#include <stdio.h>


#include "scene.h"


// From scene.c
void _freeSequence(FfxScene scene, FfxPoint worldPos, FfxNode node);


static void _updateAnimations(_Scene *scene, _Node *node) {

    _Node *animate = node->animate.node;

    // No animations
    if (animate == NULL) { return; }

    //int32_t now = xTaskGetTickCount();
    //scene->tick = now;
    int32_t now = scene->tick;

    _Node *lastAnimate = NULL;

    uint16_t stopType = FfxSceneActionStopNormal;

    while (animate) {
        int32_t duration = animate->a.i32;
        int32_t endTime = animate->b.i32;

        _Node *prop = animate->nextNode;
        _Node *nextAnimate = prop->nextNode;

        if (stopType == FfxSceneActionStopNormal) { stopType = animate->pos.y; }
        if (stopType) { endTime = now; }

        if (stopType != FfxSceneActionStopCurrent) {
            // Compute the curve-adjusted t
            // fixed_ffxt t = FM_1 - divfx((endTime - now) << 16, duration << 16);
            fixed_ffxt t = FM_1 - (tofx(endTime - now) / duration);
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

void _groupSequence(FfxScene _scene, FfxPoint worldPos, FfxNode _node) {
    _Scene *scene = _scene;
    _Node *node = _node;

    FfxPoint pos = node->pos;
    worldPos.x += pos.x;
    worldPos.y += pos.y;

    // I have children; visit them each
    _Node *lastChild = NULL;
    _Node *child = node->a.ptr;
    while (child) {
        _Node *nextChild = child->nextNode;

        if (child->func.sequenceFunc == _freeSequence) {
            // Remove child

            // The tail is the child; point the tail to the previous child (NULL if firstChild)
            if (node->b.ptr == child) { node->b.ptr = lastChild; }

            if (lastChild) {
                lastChild->nextNode = child->nextNode;
            } else {
                node->a.ptr = child->nextNode;
            }

            _freeNode(scene, child);

        } else {
            // Update the animations
            _updateAnimations(scene, child);

            // Sequence the child
            child->func.sequenceFunc(_scene, worldPos, child);
            lastChild = child;
        }

        child = nextChild;
    }
}

static FfxNode _debug(FfxNode node, FfxNodeFunc func, char *descr,
  size_t length) {

    if (func.sequenceFunc == _groupSequence) {
        FfxPoint *pos = ffx_scene_nodePosition(node);
        FfxProperty *a = ffx_scene_nodePropertyA(node);
        snprintf(descr, length,
          "Group Sequence Node pos=(%d, %d)", pos->x, pos->y);
        return a->ptr;
    }

    return NULL;
}

FfxNode ffx_scene_createGroup(FfxScene scene) {
    REGISTER_DEBUG(_debug);

    FfxProperty a, b;
    a.ptr = b.ptr = NULL;
    return ffx_scene_createNode(scene, _groupSequence, a, b);
}

/**
 *  Adds a child to a parent GroupNode (created using scene_createGroup).
 *
 *  NOTE: Child MUST NOT already have a parent
 */
void ffx_scene_appendChild(FfxNode _parent, FfxNode _child) {
    if (_parent == NULL || _child == NULL) { return; }

    _Node *parent = _parent;
    _Node *child = _child;

    child->nextNode = NULL;

    if (parent->a.ptr == NULL) {
        // First child
        parent->a.ptr = parent->b.ptr = child;

    } else {
        // Append to the child to the end
        _Node *lastChild = parent->b.ptr;
        lastChild->nextNode = child;
        parent->b.ptr = child;
    }
}

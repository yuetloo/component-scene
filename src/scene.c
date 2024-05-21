#include <stddef.h>
#include <stdio.h>
#include <string.h>

// For tick
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "scene.h"


static const int16_t yHidden = 0x7fff;

const FfxPoint PointZero = { .x = 0, .y = 0 };
const FfxPoint PointHidden = { .x = 0, .y = yHidden };
const FfxSize SizeZero = { .width = 0, .height = 0 };


// The node.a property is not cleared the caller must update this
static _Node* _addNode(_Scene *scene, FfxPoint pos) {
    _Node *free = scene->nextFree;
    if (free == NULL) {
        printf("No free nodes...\n");
        return NULL;
    }

    scene->nextFree = free->nextNode;
    free->pos = pos;
    // @TOOD: don't do this and copy in sequence to avoid unecessary copies?
    free->b.ptr = NULL;
    free->nextNode = NULL;
    free->animate.node = NULL;

    return free;
}

void _freeNode(_Scene *scene, _Node *node) {
    node->nextNode = scene->nextFree;
    scene->nextFree = node;

    // @TODO: Free animationHead
}

FfxNode ffx_scene_createNode(FfxScene _scene, FfxSequenceFunc sequenceFunc,
  FfxProperty a, FfxProperty b) {
    _Scene *scene = _scene;
    _Node *node = _addNode(scene, PointZero);
    node->func.sequenceFunc = sequenceFunc;
    node->a = a;
    node->b = b;
    return node;
}

// @TODO: return uint32?
FfxNode ffx_scene_createAnimationNode(FfxScene _scene, FfxNode _node, FfxProperty a, FfxProperty b, uint32_t duration, FfxAnimateFunc animateFunc, FfxCurveFunc curveFunc, FfxSceneAnimationCompletion onComplete) {
    _Scene *scene = _scene;
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

FfxNode ffx_scene_createRenderNode(FfxScene _scene, FfxNode _sceneNode,
  FfxPoint pos, FfxRenderFunc renderFunc) {
    _Scene *scene = _scene;

    _Node *sceneNode = _sceneNode;

    pos.x += sceneNode->pos.x;
    pos.y += sceneNode->pos.y;

    _Node *node = _addNode(scene, pos);
    if (node == NULL) { return NULL; }

    node->func.renderFunc = renderFunc;
    node->a = sceneNode->a;
    node->b = sceneNode->b;

    // Add the RenderNode to the render list
    if (scene->renderHead == NULL) {
        // First render node; it is the head and the tail
        scene->renderHead = scene->renderTail = node;

    } else {
        // Add this to the end of the list and make it the end
        scene->renderTail->nextNode = node;
        scene->renderTail = node;
    }

    return node;
}


FfxScene ffx_scene_init(uint32_t nodeCount) {
    // We need at least a root and render head
    if (nodeCount < 2) { return NULL; }

    _Scene *scene = malloc(sizeof(_Scene));
    if (scene == NULL) { return NULL; }
    memset(scene, 0, sizeof(_Scene));

    _Node *nodes = malloc(nodeCount * sizeof(_Node));
    if (nodes == NULL) {
        free(scene);
        return NULL;
    }
    memset(nodes, 0, nodeCount * sizeof(_Node));
    scene->nodes = nodes;

    // Create a linked list of all nodes linking to the next free
    for (uint32_t i = 0; i < nodeCount - 1; i++) {
        nodes[i].nextNode = &nodes[i + 1];
    }

    // The next free Node to acquire
    scene->nextFree = &nodes[0];

    scene->tick = xTaskGetTickCount();

    // Set up the root node and render list
    scene->root = ffx_scene_createGroup(scene);
    scene->renderHead = NULL;
    scene->renderTail = NULL;

    return scene;
}


FfxNode ffx_scene_root(FfxScene _scene) {
    _Scene *scene = _scene;
    return scene->root;
}


uint32_t ffx_scene_sequence(FfxScene _scene) {
    _Scene *scene = _scene;

    // Recycle the previous render list
    if (scene->renderHead != NULL) {
        scene->renderTail->nextNode = scene->nextFree;
        scene->nextFree = scene->renderHead;
        scene->renderHead = scene->renderTail = NULL;
    }

    // Sequence all the nodes
    scene->root->func.sequenceFunc(scene, PointZero, scene->root);

    return 1; // @TODO: return void?
}


void _freeSequence(FfxScene scene, FfxPoint worldPos, FfxNode node) {
    printf("WARNING: running free node\n");
}

void ffx_scene_nodeFree(FfxNode _node) {
    _Node *node = _node;
    node->func.sequenceFunc = _freeSequence;
}


void ffx_scene_render(FfxScene _scene, uint8_t *fragment, int32_t y0, int32_t height) {
    _Scene *scene = _scene;

    _Node *node = scene->renderHead;
    while (node) {
        node->func.renderFunc(node->pos, node->a, node->b, (uint16_t*)fragment, y0, height);
        node = node->nextNode;
    }
}


//////////////////////////
// Node

void ffx_scene_nodeSetPosition(FfxNode _node, FfxPoint pos) {
    _Node *node = _node;
    if (node == NULL) { return; }
    node->pos = pos;
}

FfxPoint ffx_scene_nodePosition(FfxNode _node) {
    _Node *node = _node;
    if (node == NULL) { return PointZero; }
    return node->pos;
}

// Setting a property of an invalid (NULL) node, will update
// this rather than crash. It gets reset whenever it is
// returned but could still contain junk at any point.
static FfxProperty junkProperty;

FfxProperty* ffx_scene_nodePropertyA(FfxNode _node) {
    _Node *node = _node;
    if (node == NULL) {
        printf("[scene] Warning: updating bad proeprty\n");
        junkProperty.ptr = NULL;
        return &junkProperty;
    }
    return &node->a;
}

FfxProperty* ffx_scene_nodePropertyB(FfxNode _node) {
    _Node *node = _node;
    if (node == NULL) {
        printf("[scene] Warning: updating bad proeprty\n");
        junkProperty.ptr = NULL;
        return &junkProperty;
    }
    return &node->b;
}


//////////////////////////
// Node animation

// @TODO: drop SceneContext from this?
static void _nodeAnimatePositionHoriz(FfxNode _node, fixed_t t, FfxProperty p0, FfxProperty p1) {
    _Node *node = _node;
    node->pos.x = p0.point.x + scalarfx(p1.point.x - p0.point.x, t);
}

static void _nodeAnimatePositionVert(FfxNode _node, fixed_t t, FfxProperty p0, FfxProperty p1) {
    _Node *node = _node;
    node->pos.y = p0.point.y + scalarfx(p1.point.y - p0.point.y, t);
}

static void _nodeAnimatePosition(FfxNode _node, fixed_t t, FfxProperty p0, FfxProperty p1) {
    _Node *node = _node;

    FfxPoint result = p0.point;
    result.x += scalarfx(p1.point.x - p0.point.x, t);
    result.y += scalarfx(p1.point.y - p0.point.y, t);
    node->pos = result;
}

uint32_t ffx_scene_nodeAnimatePosition(FfxScene scene, FfxNode _node, FfxPoint target, uint32_t duration, FfxCurveFunc curve, FfxSceneAnimationCompletion onComplete) {
    //_Scene *scene = _scene;
    _Node *node = _node;

    FfxAnimateFunc animateFunc = _nodeAnimatePosition;
    if (node->pos.x == target.x) {
        animateFunc = _nodeAnimatePositionVert;
    } else if (node->pos.y == target.y) {
        animateFunc = _nodeAnimatePositionHoriz;
    }

    FfxProperty a, b;
    a.point = node->pos;
    b.point = target;

    _Node *animate = ffx_scene_createAnimationNode(scene, node, a, b,
      duration, animateFunc, curve, onComplete);
    if (animate == NULL) { return 0; }

    //animate->nextNode->a.point = node->pos;
    //animate->nextNode->b.point = target;

    return 1;
}

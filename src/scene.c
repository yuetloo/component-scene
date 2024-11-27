#include <stddef.h>
#include <stdio.h>
#include <string.h>

// For tick
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "scene.h"


const FfxPoint PointZero = { .x = 0, .y = 0 };


//////////////////////////
// Creating nodes


static _Node* _addNode(_Scene *scene, FfxPoint pos) {
    _Node *free = scene->nextFree;
    if (free == NULL) {
        printf("No free nodes...\n");
        return NULL;
    }

    scene->nextFree = free->nextNode;
    //free->tag = 0;
    free->pos = pos;
    free->nextNode = NULL;
    free->animate.node = NULL;

    return free;
}

// Only call _freeNode on SequenceNodes or RenderNodes; it is not
// safe to call on AnimationNodes (except for the rare case during
// AnimationNode creation, before the node has a `.animate` value).
void _freeNode(_Scene *scene, _Node *node) {

    // Recycle any animation nodes
    _Node *head = node->animate.node;
    if (head) {
        // Find the tail
        _Node *tail = head;
        while (tail && tail->nextNode) {
            tail = tail->nextNode;
        }

        // Insert the animation sequence at the front of the free list
        tail->nextNode = scene->nextFree;
        scene->nextFree = head;
    }

    // Recycle the node itself
    node->nextNode = scene->nextFree;
    scene->nextFree = node;
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
FfxNode ffx_scene_createAnimationNode(FfxScene _scene, FfxNode _node, FfxProperty start, FfxProperty end, uint32_t duration, FfxAnimateFunc animateFunc, FfxCurveFunc curveFunc, FfxSceneAnimationCompletion onComplete) {
    _Scene *scene = _scene;
    _Node *node = _node;

    _Node* animate = _addNode(scene, PointZero);
    if (animate == NULL) { return NULL; }

    _Node *prop = _addNode(scene, PointZero);
    if (prop == NULL) {
        // It is safe to call free on this animation node because
        // its .animate has been cleared
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

    prop->a = start;
    prop->b = end;

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


//////////////////////////
// Life-cycle

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

void _freeSequence(FfxScene scene, FfxPoint worldPos, FfxNode node) {
    printf("WARNING: running free node\n");
}

void ffx_scene_nodeFree(FfxNode _node) {
    _Node *node = _node;
    node->func.sequenceFunc = _freeSequence;
}


//////////////////////////
// Sequencing and Rendering

uint32_t ffx_scene_sequence(FfxScene _scene) {
    _Scene *scene = _scene;

    scene->tick = xTaskGetTickCount();

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

void ffx_scene_render(FfxScene _scene, uint8_t *fragment, int32_t y0, int32_t height) {
    _Scene *scene = _scene;

    _Node *node = scene->renderHead;
    while (node) {
        node->func.renderFunc(node->pos, node->a, node->b, (uint16_t*)fragment, y0, height);
        node = node->nextNode;
    }
}


//////////////////////////
// Animations

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

void ffx_scene_advanceAnimations(FfxNode _node, uint32_t advance) {
    _Node *node = (_Node*)_node;
    if (node == NULL || node->animate.node == NULL) { return; }

    node->animate.node->pos.x = advance;
    node->animate.node->pos.y = 0xfe;
}


//////////////////////////
// Node

/*
int32_t ffx_scene_nodeTag(FfxNode _node) {
    _Node *node = _node;
    if (node == NULL) { return 0; }
    return node->tag;
}
*/
/*
int32_t ffx_scene_nodeSetTag(FfxNode _node, int32_t tag) {
    _Node *node = _node;
    if (node == NULL) {
        printf("[scene] Warning: updating bad proeprty (tag)\n");
        return 0;
    }
    int32_t oldTag = node->tag;
    node->tag = tag;
    return oldTag;
}
*/
// Setting a property of an invalid (NULL) node, will update
// this rather than crash. It gets reset whenever it is
// returned but could still contain junk at any point.
static FfxProperty junkProperty;

FfxProperty* ffx_scene_junkProperty(char *nodeName, char *propName) {
    printf("[%s] Warning: updating bad property (%s)\n", nodeName, propName);
    junkProperty.ptr = NULL;
    return &junkProperty;
}

FfxPoint* ffx_scene_nodePosition(FfxNode _node) {
    _Node *node = _node;
    if (node == NULL) {
        return &ffx_scene_junkProperty("node", "pos")->point;
    }
    return &node->pos;
}

void ffx_scene_nodeSetPosition(FfxNode node, FfxPoint point) {
    FfxPoint *p = ffx_scene_nodePosition(node);
    p->x = point.x;
    p->y = point.y;
}

FfxProperty* ffx_scene_nodePropertyA(FfxNode _node) {
    _Node *node = _node;
    if (node == NULL) {
        return ffx_scene_junkProperty("node", "a");
    }
    return &node->a;
}

FfxProperty* ffx_scene_nodePropertyB(FfxNode _node) {
    _Node *node = _node;
    if (node == NULL) {
        return ffx_scene_junkProperty("node", "b");
    }
    return &node->b;
}


//////////////////////////
// Node Animation
/*
static void _nodeAnimatePositionHoriz(FfxNode node, fixed_t t, FfxProperty p0, FfxProperty p1) {
    //_Node *node = _node;
    //node->pos.x = p0.point.x + scalarfx(p1.point.x - p0.point.x, t);
    FfxPoint *pos = ffx_scene_nodePosition(node);
    pos->x = p0.point.x + scalarfx(p1.point.x - p0.point.x, t);
}

static void _nodeAnimatePositionVert(FfxNode node, fixed_t t, FfxProperty p0, FfxProperty p1) {
    //_Node *node = _node;
    //node->pos.y = p0.point.y + scalarfx(p1.point.y - p0.point.y, t);
    FfxPoint *pos = ffx_scene_nodePosition(node);
    pos->y = p0.point.y + scalarfx(p1.point.y - p0.point.y, t);
}
*/
/*
static void _nodeAnimatePosition(FfxNode node, fixed_t t, FfxProperty p0, FfxProperty p1) {
    FfxPoint *pos = ffx_scene_nodePosition(node);
    pos->x = p0.point.x + scalarfx(p1.point.x - p0.point.x, t);
    pos->y = p0.point.y + scalarfx(p1.point.y - p0.point.y, t);
*/
/*
    _Node *node = _node;
    FfxPoint result = p0.point;
    result.x += scalarfx(p1.point.x - p0.point.x, t);
    result.y += scalarfx(p1.point.y - p0.point.y, t);
    node->pos = result;
*/
//}
/*
uint32_t ffx_scene_nodeAnimatePosition(FfxScene scene, FfxNode node, FfxPoint target, uint32_t duration, FfxCurveFunc curve, FfxSceneAnimationCompletion onComplete) {
    FfxPoint *pos = ffx_scene_nodePosition(node);

    FfxAnimateFunc animateFunc = _nodeAnimatePosition;
    if (pos->x == target.x) {
        animateFunc = _nodeAnimatePositionVert;
    } else if (pos->y == target.y) {
        animateFunc = _nodeAnimatePositionHoriz;
    }

    FfxProperty start, end;
    start.point = *pos;
    end.point = target;

    _Node *animate = ffx_scene_createAnimationNode(scene, node, start, end,
      duration, animateFunc, curve, onComplete);
    if (animate == NULL) { return 0; }

    return 1;
}
*/

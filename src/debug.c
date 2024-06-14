#include <stdio.h>
#include <string.h>

#include "scene.h"


// From node-group.c
void _groupSequence(FfxScene _scene, FfxPoint worldPos, FfxNode _node);

// From scene.c
void _freeSequence(FfxScene scene, FfxPoint worldPos, FfxNode node);


static uint32_t debugCallbackCount = 0;
static FfxSceneDumpNodeFunc debugFuncs[FFX_SCENE_DEBUG];

int32_t ffx_scene_registerDebug(FfxSceneDumpNodeFunc dumpNode) {
    if (debugCallbackCount < FFX_SCENE_DEBUG) {
        debugFuncs[debugCallbackCount++] = dumpNode;
    }
    return 1;
}


static void _dumpSequenceNode(_Node *node, int32_t indent) {
    if (node == NULL) { return; }

    uint8_t padding[2 * indent + 1];
    memset(padding, 32, sizeof(padding));
    padding[2 * indent] = 0;

    if (node->func.sequenceFunc == _freeSequence) {
        printf("%s- [pending free node]\n", padding);
        return;
    }

    if (node->func.sequenceFunc == _groupSequence) {
        printf("%s- Group Sequence Node pos=(%d, %d)\n", padding,
          node->pos.x, node->pos.y);
        _dumpSequenceNode(node->a.ptr, indent + 1);

    } else {
        char output[128];
        memset(output, 0, sizeof(output));

        for (int i = 0; i < debugCallbackCount; i++) {
            FfxSceneDumpNodeFunc dumpFunc = debugFuncs[i];
            dumpFunc(node, node->func, output, sizeof(output));

            if (output[0]) {
                printf("%s- %s \n", padding, output);
                break;
            }
        }

        if (!output[0]) {
            printf("%s- [unknown sequence node]\n", padding);
        }
    }

    if (node->nextNode) {
        _dumpSequenceNode(node->nextNode, indent);
    }
}

static void _dumpRenderNode(_Node *node, int32_t indent) {
    uint8_t padding[2 * indent + 1];
    memset(padding, 32, sizeof(padding));
    padding[2 * indent] = 0;

    char output[128];
    memset(output, 0, sizeof(output));

    uint32_t found = 0;
    for (int i = 0; i < debugCallbackCount; i++) {
        FfxSceneDumpNodeFunc dumpFunc = debugFuncs[i];
        dumpFunc(node, node->func, output, sizeof(output));

        if (output[0]) {
            printf("%s- %s\n", padding, output);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("%s- [unknown render node]\n", padding);
    }
}

/*
static void _dumpNode(FfxNode *_node, int32_t indent) {
    _Node *node = _node;

    uint8_t padding[2 * indent + 1];
    memset(padding, 32, sizeof(padding));
    padding[2 * indent] = 0;

    if (node->func.sequenceFunc == _freeSequence) {
        printf("[%04lx]%s- [pending free node]\n", index, padding);
        return;
    }

    char output[128];
    memset(output, 0, sizeof(output));

    for (int i = 0; i < debugCallbackCount; i++) {
        FfxSceneDumpNodeFunc dumpFunc = debugFuncs[i];
        //FfxNode _child = 
        dumpFunc(node, node->func, output, sizeof(output));

        if (output[0]) {
            printf("[%04lx]%s- %s \n", index, padding, output);

            if (_child) {
                _Node *child = _child;
                _dumpNode(child, indent + 1);
            }
        }

        if (node->nextNode) {
            _dumpNode(node->nextNode, indent);
        }

        return;
    }

    FfxPoint *pos = ffx_scene_nodePosition(node);

    printf("[%04lx]%s- [unknown node type] pos=(%d, %d)\n", index, padding, pos->x,
      pos->y);

    if (node->nextNode) {
        _dumpNode(node->nextNode, indent);
    }
}
*/

//static void dumpNode(_Scene *scene, _Node *node, uint32_t indent) {
/*
    uint8_t padding[2 * indent + 1];
    memset(padding, 32, sizeof(padding));
    padding[2 * indent] = 0;

    if (node->func.sequenceFunc == _groupSequence) {
        printf("%s    - Group Node %ld (x=%d, y=%d)\n", padding, _getNodeIndex(scene, node), node->pos.x, node->pos.y);
        dumpNode(scene, node->a.ptr, indent + 1);

    } else if (node->func.sequenceFunc == _boxSequence) {
        printf("%s    - Box Node %ld (x=%d, y=%d, width=%d, height=%d, color=0x%04x)\n", padding,
            _getNodeIndex(scene, node), node->pos.x, node->pos.y, node->a.size.width, node->a.size.height,
            __builtin_bswap16(node->b.u32));
    } else if (node->func.sequenceFunc == _fillSequence) {
        printf("%s    - Fill Node %ld (color=0x%04x)\n", padding, _getNodeIndex(scene, node), __builtin_bswap16(node->a.u32));
    } else if (node->func.sequenceFunc == _imageSequence) {
        printf("%s    - Image Node %ld (x=%d, y=%d, width=%d, height=%d, type=0x%02x)\n", padding,
            _getNodeIndex(scene, node), node->pos.x, node->pos.y, node->b.size.width, node->b.size.height, node->a.data[0]);
    } else if (node->func.sequenceFunc == _textSequence) {
        TextInfo info = node->b.text;
        printf("%s    - Text Node %ld (x=%d, y=%d, text=@TODO, flags=%x, length=%d, color=%04x)\n", padding,
            _getNodeIndex(scene, node), node->pos.x, node->pos.y, info.flags, info.length, info.color);

    } else if (node->func.sequenceFunc == _freeSequence) {
        printf("%s    - Pending Free Node %ld\n", padding, _getNodeIndex(scene, node));

    } else if (node->func.renderFunc == _boxRender) {
        printf("%s    - Box Render Node %ld (x=%d, y=%d, width=%d, height=%d, color=0x%04x)\n", padding,
            _getNodeIndex(scene, node), node->pos.x, node->pos.y, node->a.size.width, node->a.size.height,
            __builtin_bswap16 (node->b.u32));
    } else if (node->func.renderFunc == _fillRender) {
        printf("%s    - Fill Render Node %ld (color=0x%04x)\n", padding, _getNodeIndex(scene, node), __builtin_bswap16 (node->a.u32));
    } else if (node->func.renderFunc == _imageRender) {
        printf("%s    - Image Render Node %ld (x=%d, y=%d, width=%d, height=%d, type=0x%02x)\n", padding,
            _getNodeIndex(scene, node), node->pos.x, node->pos.y, node->b.size.width, node->b.size.height, node->a.data[0]);
    } else if (node->func.renderFunc == _textRender) {
        TextInfo info = node->b.text;
        printf("%s    - Text Render Node %ld (x=%d, y=%d, text=@TODO, flags=%x, length=%d, color=%04x)\n", padding,
            _getNodeIndex(scene, node), node->pos.x, node->pos.y, info.flags, info.length, info.color);

    } else {
        printf("%s    - Unknown Node %ld (x=%d, y=%d)\n", padding, _getNodeIndex(scene, node), node->pos.x, node->pos.y);
    }

    if (node->animate.node) {
        _Node *animate = node->animate.node;
        while (animate) {
            int32_t duration = animate->a.i32;
            int32_t end = animate->b.i32;
            float t = 1.0f - ((float)(end - scene->tick) / (float)duration);
            if (t >= 1.0f) { t = 1.0f; }
            printf("%s      - Animate Node %ld (start=%ld, end=%ld, t=%f, duration=%ld)\n", padding,
                _getNodeIndex(scene, animate), end - duration, end, t, duration);
            animate = animate->nextNode->nextNode;
        }
    }

    if (node->nextNode != NULL) { dumpNode(scene, node->nextNode, indent); }
*/
//}

void ffx_scene_dump(FfxScene _scene) {
    _Scene *scene = _scene;

    uint32_t count = 0;
    _Node *cur = scene->nextFree;
    while (cur) {
        count++;
        cur = cur->nextNode;
   }

    printf("Scene: %p\n", scene);
    printf("  - tick: %ld\n", scene->tick);
    printf("  - free nodes: %ld\n", count);

    printf("  - Scene Graph:\n");
    _dumpSequenceNode(scene->root, 2);

    cur = scene->renderHead;
    if (cur) {
        printf("  - Render Sequence:\n");
        while (cur) {
            _dumpRenderNode(cur, 0);
            cur = cur->nextNode;
        }
    }
}

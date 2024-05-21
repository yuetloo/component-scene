#include <stdio.h>

#include "scene.h"

static int32_t _getNodeIndex(_Scene *scene, const _Node *node) {
    if (node == NULL) { return -1; }
    _Node *base = &scene->nodes[0];
    return (node - base);
}

static void dumpNode(_Scene *scene, _Node *node, uint32_t indent) {
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
}

void scene_dump(FfxScene _scene) {
    _Scene *scene = _scene;
    printf("Scene: %p\n", scene);
    printf("  - tick: %ld\n", scene->tick);
    printf("  - nextFree: %ld\n", _getNodeIndex(scene, scene->nextFree));

    printf("  - Scene Graph:\n");
    dumpNode(scene, scene->root, 0);

    printf("  - Render Sequence:\n");
    if (scene->renderHead) {
        dumpNode(scene, scene->renderHead, 0);
    }
}

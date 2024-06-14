
#include <stddef.h>

#include "firefly-scene-private.h"


static void _nodeAnimatePositionHoriz(FfxNode node, fixed_ffxt t, FfxProperty p0, FfxProperty p1) {
    FfxPoint *pos = ffx_scene_nodePosition(node);
    pos->x = p0.point.x + scalarfx(p1.point.x - p0.point.x, t);
}

static void _nodeAnimatePositionVert(FfxNode node, fixed_ffxt t, FfxProperty p0, FfxProperty p1) {
    FfxPoint *pos = ffx_scene_nodePosition(node);
    pos->y = p0.point.y + scalarfx(p1.point.y - p0.point.y, t);
}

static void _nodeAnimatePosition(FfxNode node, fixed_ffxt t, FfxProperty p0, FfxProperty p1) {
    FfxPoint *pos = ffx_scene_nodePosition(node);
    pos->x = p0.point.x + scalarfx(p1.point.x - p0.point.x, t);
    pos->y = p0.point.y + scalarfx(p1.point.y - p0.point.y, t);
}

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

    FfxNode animate = ffx_scene_createAnimationNode(scene, node, start, end,
      duration, animateFunc, curve, onComplete);
    if (animate == NULL) { return 0; }

    return 1;
}

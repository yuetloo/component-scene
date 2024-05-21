
#ifndef __FIREFLY_CURVES_H__
#define __FIREFLY_CURVES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdint.h>

#include "firefly-fixed.h"


/**
 *  Fixed-point curves
 *
 *  Each function takes a parametric value %%t%% and scales
 *  it to a new value based on the curve function.
 */

typedef fixed_t (*FfxCurveFunc)(fixed_t t);

fixed_t FfxCurveLinear(fixed_t t);

fixed_t FfxCurveEaseInSine(fixed_t t);
fixed_t FfxCurveEaseOutSine(fixed_t t);
fixed_t FfxCurveEaseInOutSine(fixed_t t);

fixed_t FfxCurveEaseInQuad(fixed_t t);
fixed_t FfxCurveEaseOutQuad(fixed_t t);
fixed_t FfxCurveEaseInOutQuad(fixed_t t);

fixed_t FfxCurveEaseInCubic(fixed_t t);
fixed_t FfxCurveEaseOutCubic(fixed_t t);
fixed_t FfxCurveEaseInOutCubic(fixed_t t);

fixed_t FfxCurveEaseInQuart(fixed_t t);
fixed_t FfxCurveEaseOutQuart(fixed_t t);
fixed_t FfxCurveEaseInOutQuart(fixed_t t);

fixed_t FfxCurveEaseInQuint(fixed_t t);
fixed_t FfxCurveEaseOutQuint(fixed_t t);
fixed_t FfxCurveEaseInOutQuint(fixed_t t);

fixed_t FfxCurveEaseInExpo(fixed_t t);
fixed_t FfxCurveEaseOutExpo(fixed_t t);
fixed_t FfxCurveEaseInOutExpo(fixed_t t);

fixed_t FfxCurveEaseInBack(fixed_t t);
fixed_t FfxCurveEaseOutBack(fixed_t t);
// fixed_t CurveEaseInOutBack(fixed_t t)

fixed_t FfxCurveEaseInElastic(fixed_t t);
fixed_t FfxCurveEaseOutElastic(fixed_t t);
// fixed_t CurveEaseInOutElastic(fixed_t t)

fixed_t FfxCurveEaseInBounce(fixed_t t);
fixed_t FfxCurveEaseOutBounce(fixed_t t);
fixed_t FfxCurveEaseInOutBounce(fixed_t t);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FIREFLY_CURVES_H__ */

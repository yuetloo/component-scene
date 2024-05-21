#ifndef __FIREFLY_FIXED_H__
#define __FIREFLY_FIXED_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdint.h>


typedef int32_t fixed_t;


/**
 *  Fixed-point constants
 */

extern const fixed_t FM_1;

extern const fixed_t FM_PI_2;
extern const fixed_t FM_PI;
extern const fixed_t FM_3PI_2;
extern const fixed_t FM_2PI;

extern const fixed_t FM_E;

extern const fixed_t FM_MAX;
extern const fixed_t FM_MIN;


/**
 *  Returns %%value%% as a fixed-point value with 0 as its
 *  decimals. e.g. toFx(1) == FM_1
 */
fixed_t tofx(int32_t value);

fixed_t divfx(fixed_t x, fixed_t y);
fixed_t mulfx(fixed_t x, fixed_t y);
int32_t scalarfx(int32_t scalar, fixed_t x);

fixed_t log2fx(fixed_t arg);
fixed_t exp2fx(fixed_t arg);
fixed_t powfx(fixed_t a, fixed_t b);

fixed_t sinfx(fixed_t x);
fixed_t cosfx(fixed_t x);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FIREFLY_FIXED_H__ */

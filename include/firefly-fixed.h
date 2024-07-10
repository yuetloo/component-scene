#ifndef __FIREFLY_FIXED_H__
#define __FIREFLY_FIXED_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdint.h>


typedef int32_t fixed_ffxt;


/**
 *  Fixed-point constants
 */

extern const fixed_ffxt FM_1;

extern const fixed_ffxt FM_PI_2;
extern const fixed_ffxt FM_PI;
extern const fixed_ffxt FM_3PI_2;
extern const fixed_ffxt FM_2PI;

extern const fixed_ffxt FM_E;

extern const fixed_ffxt FM_MAX;
extern const fixed_ffxt FM_MIN;


/**
 *  Returns %%value%% as a fixed-point value with 0 as its
 *  decimals. e.g. toFx(1) == FM_1
 */
fixed_ffxt tofx(int32_t value);
fixed_ffxt ratiofx(int32_t top, int32_t bottom);

fixed_ffxt divfx(fixed_ffxt x, fixed_ffxt y);
fixed_ffxt mulfx(fixed_ffxt x, fixed_ffxt y);
int32_t scalarfx(int32_t scalar, fixed_ffxt x);

fixed_ffxt log2fx(fixed_ffxt arg);
fixed_ffxt exp2fx(fixed_ffxt arg);
fixed_ffxt powfx(fixed_ffxt a, fixed_ffxt b);

fixed_ffxt sinfx(fixed_ffxt x);
fixed_ffxt cosfx(fixed_ffxt x);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FIREFLY_FIXED_H__ */

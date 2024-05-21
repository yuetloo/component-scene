#include "firefly-fixed.h"


const fixed_t FM_PI_2   = 0x19220;
const fixed_t FM_PI     = 2 * FM_PI_2;
const fixed_t FM_3PI_2  = 3 * FM_PI_2;
const fixed_t FM_2PI    = 4 * FM_PI_2;

const fixed_t FM_E      = 0x2b7e1;

const fixed_t FM_MAX    = 0x7fffffff;
const fixed_t FM_MIN    = 0x80000000;

const fixed_t FM_1      =    0x10000;


fixed_t tofx(int32_t value) {
    return value << 16;
}

static uint32_t umul32hi(uint32_t a, uint32_t b) {
    return (uint32_t)(((uint64_t)a * b) >> 32);
}

/* compute log2() with s15.16 fixed-point argument and result */
fixed_t log2fx(fixed_t arg) {
    const uint32_t a0 = (uint32_t)((1.44269476063 - 1)* (1LL << 32) + 0.5);
    const uint32_t a1 = (uint32_t)(7.2131008654833e-1 * (1LL << 32) + 0.5);
    const uint32_t a2 = (uint32_t)(4.8006370104849e-1 * (1LL << 32) + 0.5);
    const uint32_t a3 = (uint32_t)(3.5339481476694e-1 * (1LL << 32) + 0.5);
    const uint32_t a4 = (uint32_t)(2.5600972794928e-1 * (1LL << 32) + 0.5);
    const uint32_t a5 = (uint32_t)(1.5535182948224e-1 * (1LL << 32) + 0.5);
    const uint32_t a6 = (uint32_t)(6.3607925549150e-2 * (1LL << 32) + 0.5);
    const uint32_t a7 = (uint32_t)(1.2319647939876e-2 * (1LL << 32) + 0.5);
    int32_t lz;
    uint32_t approx, h, m, l, z, y, x = arg;
    lz = __builtin_clz(x);

    // Shift off integer bits and normalize fraction 0 <= f <= 1
    x = x << (lz + 1);
    y = umul32hi(x, x); // f**2
    z = umul32hi(y, y); // f**4

    // Evaluate minimax polynomial to compute log2(1+f)
    h = a0 - umul32hi(a1, x);
    m = umul32hi(a2 - umul32hi(a3, x), y);
    l = umul32hi(a4 - umul32hi(a5, x) + umul32hi(a6 - umul32hi(a7, x), y), z);
    approx = x + umul32hi (x, h + m + l);

    // Combine integer and fractional parts of result; round result
    approx = ((15 - lz) << 16) + ((((approx) >> 15) + 1) >> 1);
    return approx;
}

/* compute exp2() with s15.16 fixed-point argument and result */
fixed_t exp2fx(fixed_t arg) {
    const uint32_t a0 = (uint32_t)(6.9314718107e-1 * (1LL << 32) + 0.5);
    const uint32_t a1 = (uint32_t)(2.4022648809e-1 * (1LL << 32) + 0.5);
    const uint32_t a2 = (uint32_t)(5.5504413787e-2 * (1LL << 32) + 0.5);
    const uint32_t a3 = (uint32_t)(9.6162736882e-3 * (1LL << 32) + 0.5);
    const uint32_t a4 = (uint32_t)(1.3386828359e-3 * (1LL << 32) + 0.5);
    const uint32_t a5 = (uint32_t)(1.4629773796e-4 * (1LL << 32) + 0.5);
    const uint32_t a6 = (uint32_t)(2.0663021132e-5 * (1LL << 32) + 0.5);
    int32_t i;
    uint32_t approx, h, m, l, s, q, f, x = arg;

    // Extract integer portion; 2**i is realized as a shift at the end
    i = ((x >> 16) ^ 0x8000) - 0x8000;

    // Extract and normalize fraction f to compute 2**(f)-1, 0 <= f < 1
    f = x << 16;

    // Evaluate minimax polynomial to compute exp2(f)-1
    s = umul32hi(f, f); // f**2
    q = umul32hi(s, s); // f**4
    h = a0 + umul32hi(a1, f);
    m = umul32hi(a2 + umul32hi(a3, f), s);
    l = umul32hi(a4 + umul32hi(a5, f) + umul32hi(a6, s), q);
    approx = umul32hi(f, h + m + l);

    // combine integer and fractional parts of result; round result
    approx = ((approx >> (15 - i)) + (0x80000000 >> (14 - i)) + 1) >> 1;

    // handle underflow to 0
    approx = ((i < -16) ? 0: approx);
    return approx;
}

/* s15.16 division without rounding */
fixed_t divfx(fixed_t x, fixed_t y) {
    return ((int64_t)x * 65536) / y;
}

/* s15.16 multiplication with rounding */
fixed_t mulfx(fixed_t x, fixed_t y) {
    int32_t r;
    int64_t t = (int64_t)x * (int64_t)y;
    r = (int32_t)(uint32_t)(((uint64_t)t + (1 << 15)) >> 16);
    return r;
}


int32_t scalarfx(int32_t _scalar, fixed_t x) {
    int64_t scalar = _scalar;
    return (scalar * x) >> 16;
}

/* compute a**b for a >= 0 */
fixed_t powfx(fixed_t a, fixed_t b) {
    return exp2fx(mulfx(b, log2fx(a)));
}

// See: http://www.coranac.com/2009/07/sines/
fixed_t sinfx(fixed_t x) {

    // Normalize i to [0, 2pi)
    x = (((x % FM_2PI) + FM_2PI) % FM_2PI);

    // Account for the quadrant reflecting in the x or y axis
    int32_t ymul = 1;
    if (x >= FM_3PI_2) {
        x -= FM_3PI_2;
        x = FM_PI_2 - x;
        ymul = -1;
    } else if (x >= FM_PI) {
        x -= FM_PI;
        ymul = -1;
    } else if (x >= FM_PI_2) {
        x -= FM_PI_2;
        x = FM_PI_2 - x;
    }

    // 3rd order approximation;
    // Note: tweaked 0xf476 to 0xf475 so that FM_PI and FM_3PI_2
    // line up perfectly with 1 and -1 respectively.
    fixed_t result = mulfx(0xf475, x) - mulfx(0x2106, mulfx(mulfx(x, x), x));

    return ymul * result;
}

fixed_t cosfx(fixed_t x) {
    return sinfx(x + FM_PI_2);
}


// Fixed point maths and Various curve functions
// See: https://easings.net/


#include "firefly-curves.h"


const fixed_t FM_1_2    =     0x8000;
const fixed_t FM_1_4    =     0x4000;

const fixed_t FM_2      =    0x20000;
const fixed_t FM_10     =    0xa0000;
const fixed_t FM_20     =   0x140000;


fixed_t FfxCurveLinear(fixed_t t) { return t; }


fixed_t FfxCurveEaseInSine(fixed_t t) {
    // return 1.0f - cosf((t * M_PI) / 2.0f);
    return FM_1 - cosfx(mulfx(t, FM_PI) >> 1);
}

fixed_t FfxCurveEaseOutSine(fixed_t t) {
    // return sinf((t * M_PI) / 2.0f);
    return sinfx(mulfx(t, FM_PI) >> 1);
}

fixed_t FfxCurveEaseInOutSine(fixed_t t) {
    // return -(cosf(M_PI * t) - 1.0f) / 2.0f;
    return -(cosfx(mulfx(FM_PI, t) - FM_1) >> 1);
}


fixed_t FfxCurveEaseInQuad(fixed_t t) {
    // return t * t; 
    return mulfx(t, t);
}

fixed_t FfxCurveEaseOutQuad(fixed_t t) {
    // return 1.0f - (1.0f - t) * (1.0f - t);
    fixed_t alpha1 = FM_1 - t;
    return FM_1 - mulfx(alpha1, alpha1);
}

fixed_t FfxCurveEaseInOutQuad(fixed_t t) {
    // return (t < 0.5f) ? 2.0f * t * t:
    //     1.0f - powf(-2.0 * t + 2.0f, 2.0f) / 2.0f;
    if (t < FM_1_2) { return mulfx(t, t) << 1; }
    fixed_t alpha1 = -(t << 1) + FM_2;
    return FM_1 - (mulfx(alpha1, alpha1) >> 1);
}


fixed_t FfxCurveEaseInCubic(fixed_t t) {
    // return t * t * t;
    return mulfx(t, mulfx(t, t));
}

fixed_t FfxCurveEaseOutCubic(fixed_t t) {
    //return 1.0f - powf(1.0f - t, 3.0f);
    return FM_1 - FfxCurveEaseInCubic(FM_1 - t);

}

fixed_t FfxCurveEaseInOutCubic(fixed_t t) {
    // return (t < 0.5f) ? (4 * t * t * t): 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    if (t < FM_1_2) { return FfxCurveEaseInCubic(t) << 2; }
    return FM_1 - (FfxCurveEaseInCubic(-(t << 1) + FM_2) >> 1);
}


fixed_t FfxCurveEaseInQuart(fixed_t t) {
    //return t * t * t * t;
    fixed_t t2 = mulfx(t, t);
    return mulfx(t2, t2);
}

fixed_t FfxCurveEaseOutQuart(fixed_t t) {
    // return 1.0f - powf(1.0f - t, 4.0f);
    return FM_1 - FfxCurveEaseInQuart(FM_1 - t);
}

fixed_t FfxCurveEaseInOutQuart(fixed_t t) {
    // return (t < 0.5f) ? 8.0f * t * t * t * t: 1.0 - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
    if (t < FM_1_2) { return FfxCurveEaseInQuart(t) << 3; }
    return FM_1 - (FfxCurveEaseInQuart(-(t << 1) + FM_2) >> 1);
}


fixed_t FfxCurveEaseInQuint(fixed_t t) {
    // return t * t * t * t * t;
    fixed_t t2 = mulfx(t, t);
    return mulfx(t, mulfx(t2, t2));
}

fixed_t FfxCurveEaseOutQuint(fixed_t t) {
    // return 1.0f - powf(1.0f - t, 5);
    return FM_1 - FfxCurveEaseInQuint(FM_1 - t);
}

fixed_t FfxCurveEaseInOutQuint(fixed_t t) {
    // return (t < 0.5f) ? 16.0f * t * t * t * t * t: 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;
    if (t < FM_1_2) {
        return FfxCurveEaseInQuint(t) << 4;
    }
    return FM_1 - (FfxCurveEaseInQuint(-(t << 1) + FM_2) >> 1);

}

fixed_t FfxCurveEaseInExpo(fixed_t t) {
    // return (t == 0.0f) ? 0.0f: powf(2.0f, 10.0f * t - 10.0f);
    if (t == 0) { return 0; }
    return powfx(FM_2, mulfx(FM_10, t - FM_10));
}

fixed_t FfxCurveEaseOutExpo(fixed_t t) {
    // return (t == 1.0f) ? 1.0f: 1.0f - powf(2.0f, -10.0f * t);
    if (t == FM_1) { return FM_1; }
    return FM_1 - powfx(FM_2, mulfx(-FM_10, t));
}

fixed_t FfxCurveEaseInOutExpo(fixed_t t) {
    // return (t == 0.0f) ? 0.0f:
    //     (t == 1.0f) ? 1.0f:
    //     (t < 0.5f) ? powf(2.0f, 20.0f * t - 10.0f) / 2.0f:
    //     (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
    if (t == 0) { return 0; }
    if (t == FM_1) { return FM_1; }
    if (t < FM_1_2) { return powfx(FM_2, mulfx(-FM_20, t) + FM_10) >> 1; }
    return (FM_2 - powfx(FM_2, mulfx(-FM_20, t) + FM_10)) >> 1;
}

// float FfxCurveEaseInCirc(float t) { return 1.0f - sqrtf(1.0f - powf(t, 2.0f)); }
// float FfxCurveEaseOutCirc(float t) { return sqrtf(1.0f - powf(t - 1.0f, 2.0f)); }
// float FfxCurveEaseInOutCirc(float t) {
//     return (t < 0.5f) ? (1.0f - sqrtf(1.0f - powf(2.0f * t, 2.0f))) / 2.0f:
//         (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
// }

fixed_t FfxCurveEaseInBack(fixed_t t) {
    // const float c1 = 1.70158f;
    // const float c3 = c1 + 1.0f;
    // return c3 * t * t * t - c1 * t * t;

    const fixed_t c1 = 0x1b39b;
    const fixed_t c3 = c1 + FM_1;
    const fixed_t t2 = mulfx(t, t);
    return mulfx(mulfx(c3, t2), t) - mulfx(c1, t2);
}

fixed_t FfxCurveEaseOutBack(fixed_t t) {
    // const float c1 = 1.70158f;
    // const float c3 = c1 + 1.0f;

    // return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
    // float t_1 = t - 1.0f
    // return 1.0f + c3 * t_1 * t_1 * t_1 + c1 * t_1 * t_1;

    const fixed_t c1 = 0x1b39b;
    const fixed_t c3 = c1 + FM_1;
    const fixed_t t_1 = t - FM_1;
    const fixed_t t2 = mulfx(t_1, t_1);

    return FM_1 + mulfx(c3, mulfx(t_1, t2)) - mulfx(c1, t2);
}

// fixed_t FfxCurveEaseInOutBack(fixed_t t) {
    // return (t == 0.0f) ? 0.0f:
    //     (t == 1.0f) ? 1.0f:
    //     (t < 0.5f) ? powf(2.0, 20.0 * t - 10.0f) / 2.0f:
    //     (2.0f - powf(2.0, -20.0f * t + 10.0f)) / 2.0f;

//     if (t == 0) { return 0; }
//     if (t == 1) { return 1; }
//     if (tx < FixedHalf) {
//     }
// }

fixed_t FfxCurveEaseInElastic(fixed_t t) {
    // const float c4 = (2.0f * M_PI) / 3.0f;
    // return (t == 0.0f) ? 0.0f:
    //     (t == 1.0f) ? 1.0f:
    //     -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c4);

    const fixed_t c4 = 0x2182a;
    if (t == 0) { return 0; }
    if (t == 1) { return 1; }
    return -mulfx(powfx(FM_2, mulfx(FM_10, t) - FM_10), sinfx(mulfx(mulfx(FM_10, t) - 0xac000, c4)));
}

fixed_t FfxCurveEaseOutElastic(fixed_t t) {
    // const float c4 = (2 * M_PI) / 3;
    // return (t == 0) ? 0.0f:
    //     (t == 1) ? 1.0f:
    //     powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;

    const fixed_t c4 = 0x2182a;
    if (t == 0) { return 0; }
    if (t == 1) { return 1; }
    return mulfx(powfx(FM_2, mulfx(-FM_10, t)), sinfx(mulfx(mulfx(FM_10, t) - 0xc000, c4))) + FM_1;
}

// float FfxCurveEaseInOutElastic(float t) {
//     const float c5 = (2.0f * M_PI) / 4.5f;
//     return (t == 0.0f) ? 0.0f:
//         (t == 1.0f) ? 1.0f:
//         (t < 0.5f) ? -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c5)) / 2.0f:
//         (pow(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
// }

fixed_t FfxCurveEaseInBounce(fixed_t t) {
    // return 1.0f - FfxCurveEaseOutBounce(1.0f - t);
    return FM_1 - FfxCurveEaseOutBounce(FM_1 - t);
}

fixed_t FfxCurveEaseOutBounce(fixed_t t) {
    // const float n1 = 7.5625f;
    // const float d1 = 2.75f;
    // if (t < 1.0f / d1) {
    //     return n1 * t * t;
    // } else if (t < 2.0f / d1) {
    //     return n1 * (t -= 1.5f / d1) * t + 0.75f;
    // } else if (t < 2.5f / d1) {
    //     return n1 * (t -= 2.25f / d1) * t + 0.9375f;
    // } else {
    //     return n1 * (t -= 2.625f / d1) * t + 0.984375f;
    // }

    const fixed_t n1 = 0x79000;
    const fixed_t d1 = 0x2c000;
    const fixed_t d1t = mulfx(d1, t);

    if (d1t < FM_1) { return mulfx(n1, mulfx(t, t)); }

    if (d1t < FM_2) {
        t -= (FM_1 + FM_1_2);
        return mulfx(n1 * t / d1, t) + 0xc000;
    }

    if (d1t < (FM_2 + FM_1_2)) {
        t -= (FM_2 + FM_1_4);
        return mulfx(n1 * t / d1, t) + 0xf000;
    }

    t -= 0x2a000;
    return mulfx(n1 * t / d1, t) + 0xfc00;

}

fixed_t FfxCurveEaseInOutBounce(fixed_t t) {
    // return t < 0.5f ? (1.0f - FfxCurveEaseOutBounce(1.0f - 2.0f * t)) / 2.0f:
    //     (1.0f + FfxCurveEaseOutBounce(2.0f * t - 1.0f)) / 2.0f;
    if (t < FM_1_2) { return (FM_1 - FfxCurveEaseOutBounce(FM_1 - (t << 1))) >> 1; }
    return (FM_1 + FfxCurveEaseOutBounce((t << 1) - FM_1)) >> 1;
}

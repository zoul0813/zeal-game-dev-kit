#ifndef ZGDK_CAMERA_WRAP_H
#define ZGDK_CAMERA_WRAP_H
#include <stdint.h>

/* v mod period, for period 1280 or 640 times a power of two (up to 1280 << 5).
 * 1280 << 5 == 640 << 6 == 40960: subtracting halving multiples needs no
 * division runtime and at most seven compares for any 16-bit value. */
static inline uint16_t camera_wrap(uint16_t v, uint16_t period) {
    uint16_t step = 40960U;
    do {
        if (v >= step) v -= step;
        step >>= 1;
    } while (step >= period);
    return v;
}
#endif

#include "zgdk/rect.h"

const uint8_t rect_collide(Rect *src, Rect *dst) {
    if (!src->w || !src->h || !dst->w || !dst->h) return 0;

    /* x/y are the right/bottom edges. Subtract ordered endpoints rather than
     * forming left/top coordinates, which may underflow on the 16-bit target. */
    uint8_t horizontal = src->x >= dst->x
        ? src->x - dst->x < src->w
        : dst->x - src->x < dst->w;
    uint8_t vertical = src->y >= dst->y
        ? src->y - dst->y < src->h
        : dst->y - src->y < dst->h;

    return horizontal && vertical;
}

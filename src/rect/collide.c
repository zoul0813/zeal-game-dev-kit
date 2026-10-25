#include "zgdk/rect.h"

const uint8_t rect_collide(Rect *src, Rect *dst) {
    uint8_t left = (src->x + src->w > dst->x) && (src->x + src->w < dst->x + dst->w);
    uint8_t right = (src->x > dst->x) && (src->x < dst->x + dst->w);
    uint8_t top = (src->y + src->h > dst->y) && (src->y + src->h < dst->y + dst->h);
    uint8_t bottom = (src->y > dst->y) && (src->y < dst->y + dst->h);

    return (left || right) && (top || bottom);
}

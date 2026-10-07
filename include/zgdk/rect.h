#include <stdint.h>
#include "zgdk/types.h"

#ifndef RECT_H
#define RECT_H



static const uint16_t rect_top(Rect *rect) {
  return rect->y - rect->h;
}
static const uint16_t rect_bottom(Rect *rect) {
  return rect->y;
}
static const uint16_t rect_left(Rect *rect) {
  return rect->x - rect->w;
}
static const uint16_t rect_right(Rect *rect) {
  return rect->x;
}

/* Rect coordinates are exclusive bottom/right edges. Touching edges do not
 * overlap. Compare edge distances to avoid unsigned origin underflow. */
static const uint8_t rect_collide(Rect *src, Rect *dst) {
    if (src->w == 0 || src->h == 0 || dst->w == 0 || dst->h == 0)
        return false;
    uint8_t horizontal = src->x <= dst->x
        ? dst->x - src->x < dst->w
        : src->x - dst->x < src->w;
    uint8_t vertical = src->y <= dst->y
        ? dst->y - src->y < dst->h
        : src->y - dst->y < src->h;
    return horizontal && vertical;
}

#endif
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

static const uint8_t rect_collide(Rect *src, Rect *dst) {
    uint8_t left = (src->x + src->w > dst->x) && (src->x + src->w < dst->x + dst->w);
    uint8_t right = (src->x > dst->x) && (src->x < dst->x + dst->w);
    uint8_t top = (src->y + src->h > dst->y) && (src->y + src->h < dst->y + dst->h);
    uint8_t bottom = (src->y > dst->y) && (src->y < dst->y + dst->h);

    return (left || right) && (top || bottom);
}

#endif
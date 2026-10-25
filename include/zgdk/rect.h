#include <stdint.h>
#include "zgdk/types.h"

#ifndef RECT_H
#define RECT_H



const uint16_t rect_top(Rect *rect);
const uint16_t rect_bottom(Rect *rect);
const uint16_t rect_left(Rect *rect);
const uint16_t rect_right(Rect *rect);

const uint8_t rect_collide(Rect *src, Rect *dst);

#endif

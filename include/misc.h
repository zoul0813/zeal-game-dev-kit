#include <stdint.h>

#ifndef MISC_H
#define MISC_H

#define DIRECTION_NONE      (0)
#define DIRECTION_LEFT      (-1)
#define DIRECTION_RIGHT     (1)
#define DIRECTION_UP        (-1)
#define DIRECTION_DOWN      (1)

typedef enum {
  EdgeNone          = 0,
  EdgeTop           = 1,
  EdgeBottom        = 2,
  EdgeLeft          = 4,
  EdgeRight         = 8,
  EdgeContained     = 16, // inside
} Edge;

/**
 * This represents Sprite Space
 * (X - 16, Y - 16) coordinates
 */
typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} Rect;

typedef struct {
  uint16_t x;
  uint16_t y;
} Point;

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

typedef struct {
  int8_t x;
  int8_t y;
} Direction;

#endif
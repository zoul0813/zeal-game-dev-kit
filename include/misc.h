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

typedef struct {
    uint16_t x;
    uint16_t y;
} Vector2_u16;

typedef struct {
    uint8_t x;
    uint8_t y;
} Vector2_u8;

typedef struct {
  int16_t x;
  int16_t y;
} Vector2;

typedef struct {
  int8_t x;
  int8_t y;
} Vector2_8;

typedef Vector2_8 Direction;

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

#endif
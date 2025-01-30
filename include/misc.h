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

/**
 * 16-bit Unsigned Vector2
 */
typedef struct {
    uint16_t x;
    uint16_t y;
} Vector2_u16;

/**
 * 8-bit Unsigned Vector2
 */
typedef struct {
    uint8_t x;
    uint8_t y;
} Vector2_u8;

/**
 * 16-bit Signed Vector2
 */
typedef struct {
  int16_t x;
  int16_t y;
} Vector2;

/**
 * 8-bit Signed Vector2
 */
typedef struct {
  int8_t x;
  int8_t y;
} Vector2_8;

/**
 * 16-bit Unsigned Point (Vector2_u16)
 */
typedef Vector2_u16 Point;

/**
 * 8-bit Direction (Vector2_8)
 */
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

static const uint8_t rect_collide(Rect *src, Rect *dst) {
    uint8_t left = (src->x + src->w > dst->x) && (src->x + src->w < dst->x + dst->w);
    uint8_t right = (src->x > dst->x) && (src->x < dst->x + dst->w);
    uint8_t top = (src->y + src->h > dst->y) && (src->y + src->h < dst->y + dst->h);
    uint8_t bottom = (src->y > dst->y) && (src->y < dst->y + dst->h);

    return (left || right) && (top || bottom);
}

#endif
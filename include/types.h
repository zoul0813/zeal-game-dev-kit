#include <stdint.h>

#ifndef TYPES_H
#define TYPES_H

#ifndef bool
    #define bool    uint8_t
#endif

#ifndef false
    #define false   0
#endif

#ifndef true
    #define true    1
#endif

#ifndef NULL
    #define NULL ((void *)0)
#endif

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

#endif
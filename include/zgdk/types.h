#include <stdint.h>
#include <stdbool.h>

#ifndef TYPES_H
#define TYPES_H

#ifndef NULL
#define NULL ((void*) 0)
#endif

#define SPRITE_HEIGHT       16
#define SPRITE_WIDTH        16

#define DIRECTION_NONE  (0)
#define DIRECTION_LEFT  (-1)
#define DIRECTION_RIGHT (1)
#define DIRECTION_UP    (-1)
#define DIRECTION_DOWN  (1)

#define LAYER0     0
#define LAYER1     1

typedef enum {
    EdgeNone      = 0,
    EdgeTop       = 1,
    EdgeBottom    = 2,
    EdgeLeft      = 4,
    EdgeRight     = 8,
    EdgeContained = 16, // inside
} Edge;

/**
 * 16-bit Unsigned Vector2
 */
typedef struct {
        uint16_t x;
        uint16_t y;
} Vector2_u16;

typedef struct {
        uint16_t w;
        uint16_t h;
} Size_u16;

/**
 * 8-bit Unsigned Vector2
 */
typedef struct {
        uint8_t x;
        uint8_t y;
} Vector2_u8;

typedef struct {
        uint8_t w;
        uint8_t h;
} Size_u8;

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

inline void point_set(Point *point, uint16_t x, uint16_t y) {
    point->x = x;
    point->y = y;
}

inline void point_copy(Point *dst, Point *src) {
    dst->x = src->x;
    dst->y = src->y;
}

/**
 * 16-bit Unsigned Point (Size_u16)
 */
typedef Size_u16 Size;

/**
 * 8-bit Direction (Vector2_8)
 */
typedef Vector2_8 Direction;

/**
 * 16-bit X,Y,W,H with Point/Size union
 */
typedef struct {
        // Point (x/y)
        union {
                struct {
                        uint16_t x;
                        uint16_t y;
                };
                Point point;
        };

        // Size (w/h)
        union {
                struct {
                        uint16_t w;
                        uint16_t h;
                };
                Size size;
        };
} Rect;
#endif

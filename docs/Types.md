# ZGDK: Types

Review [zgdk/types.h](../include/zgdk/types.h) for an exhaustive list of available types/macros/constants defined by ZGDK.

## Common Constants

```C
#define SPRITE_HEIGHT       16
#define SPRITE_WIDTH        16

#define DIRECTION_NONE  (0)
#define DIRECTION_LEFT  (-1)
#define DIRECTION_RIGHT (1)
#define DIRECTION_UP    (-1)
#define DIRECTION_DOWN  (1)

#define LAYER0     0
#define LAYER1     1
```

ZGDK also provides a number of different Vector structs (`Vector2_u16`, `vector2_u8`, `Size_u8`, `Vector2`, `Vector2_8`),
along with some helpful macros for working with Vectors (`vec2_copy`, `vec2_copy_pointer`, `vec2_set`, `vec2_set_pointer`, `vec2_direction`).

In addition to Vectors, ZGDK defines a simple `Rect` struct that contains a `Point` and `Size` member

The `Rect` can be used as `rect.w` or `rect.size.w`, and allows you to pass either the single value, or a pointer to the `Size` struct.

```C
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
```

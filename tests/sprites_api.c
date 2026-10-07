/* Real public C ABI, including the eight-byte by-value sprite argument. */
#include <stddef.h>
#include <zgdk/sprites.h>

/* Assembly layout and ABI assumptions must fail loudly if the SDK changes. */
typedef char sprite_size_is_8[sizeof(gfx_sprite) == 8 ? 1 : -1];
typedef char sprite_x_at_2[offsetof(gfx_sprite, x) == 2 ? 1 : -1];
typedef char point_size_is_4[sizeof(Point) == 4 ? 1 : -1];
volatile uint16_t test_arena, test_ctx, test_result;
volatile uint8_t test_size, test_error;
gfx_sprite test_sprite;
Point test_point;
void test_init(void) { test_error = sprites_register_arena((gfx_sprite *)test_arena, test_size); }
void test_register(void) { test_result = (uint16_t)sprites_register(test_sprite); }
void test_clear(void) { sprites_deregister(); }
void test_render(void) { test_error = sprites_render((gfx_context *)test_ctx); }
void test_get_point(void) { sprite_get_point(&test_sprite, &test_point); }
void test_set_point(void) { sprite_set_point(&test_sprite, &test_point); }
void test_set_xy(void) { sprite_set_xy(&test_sprite, 0x1234, 0x5678); }

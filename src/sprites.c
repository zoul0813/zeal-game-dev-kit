#include <string.h>
#include "zgdk/sprites.h"

static gfx_sprite *ARENA;
static uint8_t ARENA_SIZE;
static uint8_t registered;

gfx_error sprites_register_arena(gfx_sprite* arena, uint8_t size) {
    ARENA = arena;
    ARENA_SIZE = size;
    registered = 0;

    return GFX_SUCCESS;
}

gfx_sprite* sprites_register(gfx_sprite sprite) {
    if(registered >= (ARENA_SIZE - 1)) return NULL;

    gfx_sprite* s = &ARENA[registered];
    memcpy(s, &sprite, sizeof(gfx_sprite));
    registered++;

    return s;
}

void sprites_deregister(void) {
    for(uint8_t i = 0; i < ARENA_SIZE; i++) {
        ARENA[i].tile = 0;
    }
}

gfx_error sprites_render(gfx_context* ctx) {
    return gfx_sprite_render_array(ctx, 0, ARENA, registered);
}

inline void sprite_get_point(gfx_sprite *sprite, Point *point) {
    point->x = sprite->x;
    point->y = sprite->y;
}

inline void sprite_set_point(gfx_sprite *sprite, Point *point) {
    sprite->x = point->x;
    sprite->y = point->y;
}

inline void sprite_set_xy(gfx_sprite *sprite, uint16_t x, uint16_t y) {
    sprite->x = x;
    sprite->y = y;
}
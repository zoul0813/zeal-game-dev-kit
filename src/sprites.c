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

gfx_error sprites_render(gfx_context* ctx) {
    return gfx_sprite_render_array(ctx, 0, ARENA, registered);
}
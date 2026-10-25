#include <core.h>
#include "zgdk/sprites.h"

static gfx_sprite *ARENA;
static uint8_t ARENA_SIZE;
static uint8_t registered;
static uint8_t rendered;

gfx_error sprites_register_arena(gfx_sprite* arena, uint8_t size) {
    if (arena == NULL || size == 0 || size > GFX_SPRITES_COUNT)
        return GFX_INVALID_ARG;

    ARENA = arena;
    ARENA_SIZE = size;
    registered = 0;
    rendered = 0;

    return GFX_SUCCESS;
}

gfx_sprite* sprites_register(gfx_sprite sprite) {
    if(registered >= ARENA_SIZE) return NULL;

    gfx_sprite* s = &ARENA[registered];
    mem_cpy(s, &sprite, sizeof(gfx_sprite));
    registered++;

    return s;
}

void sprites_deregister(void) {
    /* x = 0 puts the sprite entirely off the left edge, even if tile 0 is
     * opaque or the sprite previously used the tall-sprite option. */
    if (ARENA != NULL) mem_set(ARENA, 0, ARENA_SIZE * sizeof(gfx_sprite));
    registered = 0;
}

gfx_error sprites_render(gfx_context* ctx) {
    /* Cleared slots must reach VRAM before we stop rendering them. */
    uint8_t count = registered > rendered ? registered : rendered;
    gfx_error err = gfx_sprite_render_array(ctx, 0, ARENA, count);
    if (err == GFX_SUCCESS) rendered = registered;
    return err;
}

void sprite_get_point(gfx_sprite *sprite, Point *point) {
    point->x = sprite->x;
    point->y = sprite->y;
}

void sprite_set_point(gfx_sprite *sprite, Point *point) {
    sprite->x = point->x;
    sprite->y = point->y;
}

void sprite_set_xy(gfx_sprite *sprite, uint16_t x, uint16_t y) {
    sprite->x = x;
    sprite->y = y;
}

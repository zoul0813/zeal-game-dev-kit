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

    return GFX_SUCCESS;
}

gfx_sprite* sprites_register(gfx_sprite sprite) {
    if(ARENA == NULL || registered >= ARENA_SIZE) return NULL;

    gfx_sprite* s = &ARENA[registered];
    mem_cpy(s, &sprite, sizeof(gfx_sprite));
    registered++;

    return s;
}

void sprites_deregister(void) {
    if (ARENA != NULL)
        mem_set(ARENA, 0, (uint16_t)ARENA_SIZE * sizeof(gfx_sprite));
    registered = 0;
}

gfx_error sprites_render(gfx_context* ctx) {
    if (ctx == NULL || ARENA == NULL)
        return GFX_INVALID_ARG;
    gfx_error err;
    if (registered != 0) {
        err = gfx_sprite_render_array(ctx, 0, ARENA, registered);
        if (err != GFX_SUCCESS)
            return err;
    }
    /* Clear old hardware slots even after switching to a smaller arena. */
    gfx_sprite hidden = {0};
    for (uint8_t i = registered; i < rendered; i++) {
        err = gfx_sprite_render_array(ctx, i, &hidden, 1);
        if (err != GFX_SUCCESS)
            return err;
    }
    rendered = registered;
    return GFX_SUCCESS;
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
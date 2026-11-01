#include "internal.h"

gfx_error sprites_register_arena(gfx_sprite* arena, uint8_t size) {
    if (arena == NULL || size == 0 || size > GFX_SPRITES_COUNT)
        return GFX_INVALID_ARG;

    sprites_arena = arena;
    sprites_arena_size = size;
    sprites_registered = 0;
    sprites_rendered = 0;

    return GFX_SUCCESS;
}

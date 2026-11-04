#include <zgdk/sprites.h>
static gfx_sprite arena[16];
static gfx_sprite *reserve(void) {
    gfx_sprite hidden = {0};
    if (sprites_register_arena(arena, 16) != GFX_SUCCESS) return NULL;
    return sprites_register(hidden);
}

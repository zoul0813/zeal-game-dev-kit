#include <core.h>
#include "internal.h"

void sprites_deregister(void) {
    /* x = 0 puts the sprite entirely off the left edge, even if tile 0 is
     * opaque or the sprite previously used the tall-sprite option. */
    if (sprites_arena != NULL) mem_set(sprites_arena, 0, sprites_arena_size * sizeof(gfx_sprite));
    sprites_registered = 0;
}

#include <core.h>
#include "internal.h"

gfx_sprite* sprites_register(gfx_sprite sprite) {
    if(sprites_registered >= sprites_arena_size) return NULL;

    gfx_sprite* s = &sprites_arena[sprites_registered];
    mem_cpy(s, &sprite, sizeof(gfx_sprite));
    sprites_registered++;

    return s;
}

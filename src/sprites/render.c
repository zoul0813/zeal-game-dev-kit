#include "internal.h"

gfx_error sprites_render(gfx_context* ctx) {
    /* Cleared slots must reach VRAM before we stop rendering them. */
    uint8_t count = sprites_registered > sprites_rendered ? sprites_registered : sprites_rendered;
    gfx_error err = gfx_sprite_render_array(ctx, 0, sprites_arena, count);
    if (err == GFX_SUCCESS) sprites_rendered = sprites_registered;
    return err;
}

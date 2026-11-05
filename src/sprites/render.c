#include <stdint.h>
#include <zvb_gfx.h>
#include "internal.h"

_Static_assert(sizeof(gfx_sprite) == 8, "DMA sprite layout");

gfx_error sprites_render(gfx_context* ctx)
{
    if (ctx == NULL || sprites_arena == NULL) return GFX_INVALID_ARG;
    /* Cleared slots must reach VRAM before we stop rendering them. */
    uint8_t count = sprites_registered > sprites_rendered ? sprites_registered : sprites_rendered;
    gfx_error err = GFX_SUCCESS;
    if (!count || !sprites_dma_upload())
        err = gfx_sprite_render_array(ctx, 0, sprites_arena, count);
    if (err == GFX_SUCCESS)
        sprites_rendered = sprites_registered;
    return err;
}

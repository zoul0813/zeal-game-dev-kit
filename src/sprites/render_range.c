#include "internal.h"

gfx_error sprites_render_range(gfx_context* ctx, uint8_t start, uint8_t count)
{
    uint16_t end = (uint16_t)start + count;
    if (ctx == NULL || sprites_arena == NULL || end > sprites_registered)
        return GFX_INVALID_ARG;
    if (!count) return GFX_SUCCESS;

    gfx_error err = GFX_SUCCESS;
    if (!sprites_dma_upload(start, count))
        err = gfx_sprite_render_array(ctx, start, sprites_arena + start, count);
    /* A later full render must still clear every slot uploaded by range calls. */
    if (err == GFX_SUCCESS && end > sprites_rendered)
        sprites_rendered = end;
    return err;
}

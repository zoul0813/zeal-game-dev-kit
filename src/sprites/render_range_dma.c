#include "internal.h"

gfx_error sprites_render_range_dma(gfx_context* ctx, uint8_t start, uint8_t count)
{
    uint16_t end = (uint16_t)start + count;
    if (ctx == NULL || sprites_arena == NULL || end > sprites_registered)
        return GFX_INVALID_ARG;
    if (!count) return GFX_SUCCESS;

    if (!sprites_dma_upload(start, count)) return GFX_FAILURE;
    /* A later full render must still clear every slot uploaded by range calls. */
    if (end > sprites_rendered)
        sprites_rendered = end;
    return GFX_SUCCESS;
}

#include <stdint.h>
#include <zvb_gfx.h>
#include "internal.h"

gfx_error sprites_render_dma(gfx_context* ctx)
{
    if (ctx == NULL || sprites_arena == NULL) return GFX_INVALID_ARG;
    /* Cleared slots must reach VRAM before we stop rendering them. */
    uint8_t count = sprites_registered > sprites_rendered ? sprites_registered : sprites_rendered;
    if (count && !sprites_dma_upload(0, count)) return GFX_FAILURE;
    sprites_rendered = sprites_registered;
    return GFX_SUCCESS;
}

#include "zgdk/sprites.h"

gfx_error sprites_clear(gfx_context* ctx)
{
    static const gfx_sprite hidden = {0};
    uint8_t i                      = 0;
    do {
        gfx_error err = gfx_sprite_render(ctx, i, &hidden);
        if (err != GFX_SUCCESS)
            return err;
    } while (++i < GFX_SPRITES_COUNT);
    return GFX_SUCCESS;
}

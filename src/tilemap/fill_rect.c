#include "zgdk/tilemap.h"

gfx_error tilemap_fill_rect(gfx_context *ctx, uint8_t layer, uint8_t tile, Rect *rect)
{
    return tilemap_fill(ctx, layer, tile, rect->x, rect->y, rect->w, rect->h);
}

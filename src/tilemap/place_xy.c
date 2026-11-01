#include "zgdk/tilemap.h"

gfx_error tilemap_place_xy(gfx_context *ctx, uint8_t layer, uint8_t tile, uint8_t x, uint8_t y)
{
    return gfx_tilemap_place(ctx, tile, layer, x, y);
}

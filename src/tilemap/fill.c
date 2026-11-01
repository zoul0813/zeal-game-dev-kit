#include "zgdk/tilemap.h"

gfx_error tilemap_fill(gfx_context *ctx, uint8_t layer, uint8_t tile, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    gfx_error err;
    uint8_t line[80];
    uint16_t col, row;
    if(width > sizeof(line)) return ERR_INVALID_PARAMETER;
    if(height > 40) return ERR_INVALID_PARAMETER;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            line[col] = tile;
        }
        err = gfx_tilemap_load(ctx, line, width, layer, x, y + row);
        if(err != GFX_SUCCESS) return err;
    }
    return GFX_SUCCESS;
}

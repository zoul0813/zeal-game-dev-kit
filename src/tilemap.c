#include <stdint.h>
#include <zos_errors.h>
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

gfx_error tilemap_fill_rect(gfx_context *ctx, uint8_t layer, uint8_t tile, Rect *rect)
{
    return tilemap_fill(ctx, layer, tile, rect->x, rect->y, rect->w, rect->h);
}

gfx_error tilemap_place(gfx_context *ctx, uint8_t layer, uint8_t tile, Point *point)
{
    uint8_t x, y;
    x = ((uint8_t)point->x) & 0xFF;
    y = ((uint8_t)point->y) & 0xFF;

    return gfx_tilemap_place(ctx, tile, layer, x, y);
}
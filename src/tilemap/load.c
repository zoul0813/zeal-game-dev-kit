#include "zgdk/tilemap.h"

zos_err_t tilemap_load(gfx_context *ctx, Tilemap *tilemap, uint8_t *tiles,
    uint8_t width, uint8_t height, uint8_t layer, uint8_t x, uint8_t y) {
    uint8_t row;
    uint8_t *src = tiles;
    if (!ctx || !tilemap || !tiles || !width || !height || layer > 1 ||
        (uint16_t)x + width > 80 || (uint16_t)y + height > 40)
        return ERR_INVALID_PARAMETER;
    for (row = 0; row < height; row++) {
        if (gfx_tilemap_load(ctx, src, width, layer, x, y + row) != GFX_SUCCESS)
            return ERR_FAILURE;
        src += width;
    }
    tilemap->rect.x = x;
    tilemap->rect.y = y;
    tilemap->rect.w = width;
    tilemap->rect.h = height;
    return tilemap_register(tilemap, tiles);
}

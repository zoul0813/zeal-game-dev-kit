#include <stdint.h>
#include <zos_errors.h>
#include "zgdk/tilemap.h"
#include "zgdk/types.h"

#define TILEMAP_OFFSET(x,y) (((y) * _TILEMAP->rect.w) + (x))

Tilemap *_TILEMAP;

gfx_error tilemap_fill(gfx_context *ctx, uint8_t layer, uint8_t tile, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    gfx_error err;
    uint8_t line[80];
    uint16_t col, row;
    if (width == 0 || height == 0) return GFX_SUCCESS;
    if (x >= 80 || y >= 40 || width > 80 - x || height > 40 - y)
        return GFX_INVALID_ARG;

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

/* Check map-relative coordinates while they are still 16-bit. */
static int16_t tilemap_point_xy(Point *point, Point *local)
{
    if (_TILEMAP == NULL || _TILEMAP->tiles == NULL || point == NULL) return -1;
    if (point->x < _TILEMAP->rect.x) return -2;
    if (point->y < _TILEMAP->rect.y) return -3;
    local->x = point->x - _TILEMAP->rect.x;
    local->y = point->y - _TILEMAP->rect.y;
    if (local->x >= _TILEMAP->rect.w || local->x > UINT8_MAX) return -2;
    if (local->y >= _TILEMAP->rect.h || local->y > UINT8_MAX) return -3;
    return 0;
}

gfx_error tilemap_place(gfx_context *ctx, uint8_t layer, uint8_t tile, Point *point)
{
    Point local;
    if (tilemap_point_xy(point, &local) != 0) return GFX_INVALID_ARG;
    return tilemap_place_xy(ctx, layer, tile, local.x, local.y);
}

gfx_error tilemap_place_xy(gfx_context *ctx, uint8_t layer, uint8_t tile, uint8_t x, uint8_t y)
{
    if (x >= 80 || y >= 40) return GFX_INVALID_ARG;
    return gfx_tilemap_place(ctx, tile, layer, x, y);
}

zos_err_t tilemap_register(Tilemap *tilemap, uint8_t *tiles) {
    if (tilemap == NULL || tiles == NULL) return ERR_INVALID_PARAMETER;
    _TILEMAP=tilemap;
    _TILEMAP->tiles = tiles;
    return ERR_SUCCESS;
}

int16_t tilemap_get(Point *point) {
    Point local;
    int16_t err = tilemap_point_xy(point, &local);
    if (err != 0) return err;
    return tilemap_get_xy(local.x, local.y);
}

int16_t tilemap_get_xy(uint8_t x, uint8_t y) {
    if(_TILEMAP == NULL || _TILEMAP->tiles == NULL) return -1;
    if(x >= _TILEMAP->rect.w) return -2;
    if(y >= _TILEMAP->rect.h) return -3;

    uint16_t offset = TILEMAP_OFFSET(x, y);
    return _TILEMAP->tiles[offset];
}

int16_t tilemap_set(Point *point, uint8_t value) {
    Point local;
    int16_t err = tilemap_point_xy(point, &local);
    if (err != 0) return err;
    return tilemap_set_xy(local.x, local.y, value);
}

int16_t tilemap_set_xy(uint8_t x, uint8_t y, uint8_t value) {
    if(_TILEMAP == NULL || _TILEMAP->tiles == NULL) return -1;
    if(x >= _TILEMAP->rect.w) return -2;
    if(y >= _TILEMAP->rect.h) return -3;

    uint16_t offset = TILEMAP_OFFSET(x, y);
    _TILEMAP->tiles[offset] = value;
    return _TILEMAP->tiles[offset];
}

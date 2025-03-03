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
    Point map_point;
    point_copy(&map_point, point);
    map_point.x -= (_TILEMAP->rect.x);
    map_point.y -= (_TILEMAP->rect.y);

    uint8_t x, y;
    x = ((uint8_t)map_point.x) & 0xFF;
    y = ((uint8_t)map_point.y) & 0xFF;

    return gfx_tilemap_place(ctx, tile, layer, x, y);
}

gfx_error tilemap_place_xy(gfx_context *ctx, uint8_t layer, uint8_t tile, uint8_t x, uint8_t y)
{
    return gfx_tilemap_place(ctx, tile, layer, x, y);
}

zos_err_t tilemap_register(Tilemap *tilemap, uint8_t *tiles) {
    _TILEMAP=tilemap;
    _TILEMAP->tiles = tiles;
    return ERR_SUCCESS;
}

int16_t tilemap_get(Point *point) {
    // offset to map space
    Point map_point;
    point_copy(&map_point, point);
    map_point.x -= (_TILEMAP->rect.x);
    map_point.y -= (_TILEMAP->rect.y);

    return tilemap_get_xy(map_point.x, map_point.y);
}

int16_t tilemap_get_xy(uint8_t x, uint8_t y) {
    if(_TILEMAP == NULL) return -1;
    if(x >= _TILEMAP->rect.w) return -2;
    if(y >= _TILEMAP->rect.h) return -3;

    uint16_t offset = TILEMAP_OFFSET(x, y);
    return _TILEMAP->tiles[offset];
}

int16_t tilemap_set(Point *point, uint8_t value) {
    // offset to map space
    Point map_point;
    point_copy(&map_point, point);
    map_point.x -= (_TILEMAP->rect.x);
    map_point.y -= (_TILEMAP->rect.y);

    return tilemap_set_xy(map_point.x, map_point.y, value);
}

int16_t tilemap_set_xy(uint8_t x, uint8_t y, uint8_t value) {
    if(_TILEMAP == NULL) return -1;
    if(x >= _TILEMAP->rect.w) return -2;
    if(y >= _TILEMAP->rect.h) return -3;

    uint16_t offset = TILEMAP_OFFSET(x, y);
    _TILEMAP->tiles[offset] = value;
    return _TILEMAP->tiles[offset];
}

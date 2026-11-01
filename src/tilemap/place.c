#include "zgdk/tilemap.h"

gfx_error tilemap_place(gfx_context *ctx, uint8_t layer, uint8_t tile, Point *point)
{
    Point map_point;
    vec2_copy_pointer(&map_point, point);
    map_point.x -= (_TILEMAP->rect.x);
    map_point.y -= (_TILEMAP->rect.y);

    uint8_t x, y;
    x = ((uint8_t)map_point.x) & 0xFF;
    y = ((uint8_t)map_point.y) & 0xFF;

    return gfx_tilemap_place(ctx, tile, layer, x, y);
}

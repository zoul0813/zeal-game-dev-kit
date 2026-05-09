#include <core.h>
#include "zgdk/types.h"
#include "zgdk/types.h"
#include "zgdk/rect.h"
#include "zgdk/tiles.h"
#include "zgdk/tilemap/scroll.h"


void tile_at(uint8_t tx, uint8_t ty, Tile* tile)
{
    // tile pixel coords
    uint16_t tile_t = ty * TILE_HEIGHT;
    uint16_t tile_b = tile_t + TILE_HEIGHT;
    uint16_t tile_l = tx * TILE_WIDTH;
    uint16_t tile_r = tile_l + TILE_WIDTH;

    tile->x = tx;
    tile->y = ty;

    // transpose into "sprite space"
    tile->rect.x = tile_r + tilemap_scroll_get_x(0);
    tile->rect.y = tile_b + tilemap_scroll_get_y(0);
    tile->rect.w = TILE_WIDTH;
    tile->rect.h = TILE_HEIGHT;
}

bool tile_get(Rect* src, Tile* tile)
{
    // center of point
    uint16_t srcx = src->x - SPRITE_WIDTH + (src->w / 2);
    uint16_t srcy = src->y - SPRITE_HEIGHT + (src->h / 2);

    int8_t tx = (srcx >> 4); // tilemap x
    int8_t ty = (srcy >> 4); // tilemap y
    if (tx < 0 || ty < 0)
        return false;

    tile_at(tx, ty, tile);
    return true;
}

bool tile_get_edge(Rect* rect, Edge edge, Tile* tile)
{
    if(!tile_get(rect, tile)) return false;
    if((edge & EdgeLeft) && (tile->x < 1)) return false;
    if((edge & EdgeTop) && (tile->y < 1)) return false;
    if((edge & EdgeRight) && (tile->x >= 79)) return false;
    if((edge & EdgeBottom) && (tile->y >= 39)) return false;

    if(edge & EdgeLeft) tile->x--;
    else if(edge & EdgeRight) tile->x++;
    if(edge & EdgeTop) tile->y--;
    else if(edge & EdgeBottom) tile->y++;

    return true;
}

static bool tile_get_point(int16_t x, int16_t y, Tile* tile)
{
    if (x < 0 || y < 0)
        return false;

    uint8_t tx = (uint8_t)(x >> 4);
    uint8_t ty = (uint8_t)(y >> 4);

    if (tx >= (SCROLL_WIDTH >> 4) || ty >= (SCROLL_HEIGHT >> 4))
        return false;

    tile_at(tx, ty, tile);
    return true;
}

Edge tile_collide(Rect* rect, Direction* direction, Tile* tile)
{
    int16_t probe_x = (int16_t)rect->x - SPRITE_WIDTH + (rect->w / 2);
    int16_t probe_y = (int16_t)rect->y - SPRITE_HEIGHT + (rect->h / 2);
    Edge edge = EdgeNone;

    if (direction->x > 0) {
        probe_x = (int16_t)rect->x - SPRITE_WIDTH + rect->w;
        edge |= EdgeLeft;
    } else if (direction->x < 0) {
        probe_x = (int16_t)rect->x - SPRITE_WIDTH;
        edge |= EdgeRight;
    }

    if (direction->y > 0) {
        probe_y = (int16_t)rect->y - SPRITE_HEIGHT + rect->h;
        edge |= EdgeTop;
    } else if (direction->y < 0) {
        probe_y = (int16_t)rect->y - SPRITE_HEIGHT;
        edge |= EdgeBottom;
    }

    if (edge == EdgeNone)
        return EdgeNone;

    if (!tile_get_point(probe_x, probe_y, tile))
        return EdgeNone;

    return edge;
}

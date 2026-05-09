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

bool tile_get_at_point(int16_t x, int16_t y, Tile* tile)
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

void tile_collide_ex(Rect* rect, Direction* direction, TileCollision* collision)
{
    mem_set(collision, 0, sizeof(TileCollision));
    int16_t center_x = (int16_t)rect->x - SPRITE_WIDTH + (rect->w / 2);
    int16_t center_y = (int16_t)rect->y - SPRITE_HEIGHT + (rect->h / 2);
    int16_t edge_x   = center_x;
    int16_t edge_y   = center_y;

    if (direction->x > 0) {
        edge_x = (int16_t)rect->x - SPRITE_WIDTH + rect->w;
        collision->edge_h = EdgeLeft;
    } else if (direction->x < 0) {
        edge_x = (int16_t)rect->x - SPRITE_WIDTH;
        collision->edge_h = EdgeRight;
    }

    if (direction->y > 0) {
        edge_y = (int16_t)rect->y - SPRITE_HEIGHT + rect->h;
        collision->edge_v = EdgeTop;
    } else if (direction->y < 0) {
        edge_y = (int16_t)rect->y - SPRITE_HEIGHT;
        collision->edge_v = EdgeBottom;
    }

    collision->edge = collision->edge_h | collision->edge_v;

    if (collision->edge_h != EdgeNone)
        collision->hit_h = tile_get_at_point(edge_x, center_y, &collision->tile_h);
    if (collision->edge_v != EdgeNone)
        collision->hit_v = tile_get_at_point(center_x, edge_y, &collision->tile_v);
    if (collision->edge != EdgeNone)
        collision->hit_c = tile_get_at_point(edge_x, edge_y, &collision->tile_c);

    collision->edge_c = collision->edge;

}

Edge tile_collide(Rect* rect, Direction* direction, Tile* tile)
{
    TileCollision collision;
    tile_collide_ex(rect, direction, &collision);

    if (collision.hit_c) {
        mem_cpy(tile, &collision.tile_c, sizeof(Tile));
        return collision.edge_c;
    }
    if (collision.hit_v) {
        mem_cpy(tile, &collision.tile_v, sizeof(Tile));
        return collision.edge_v;
    }
    if (collision.hit_h) {
        mem_cpy(tile, &collision.tile_h, sizeof(Tile));
        return collision.edge_h;
    }

    return EdgeNone;
}

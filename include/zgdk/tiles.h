#include <stdint.h>
#include "zgdk/types.h"

#ifndef TILES_H
#define TILES_H

#define TILE_SIZE     (16U * 16U)
#define TILE_WIDTH    16U
#define TILE_HEIGHT   16U
#define TILE_HALF     8U
#define TILE_QUARTER  4U
#define SCROLL_WIDTH  1280U
#define SCROLL_HEIGHT 640U


typedef struct {
        union {
                struct {
                        int16_t x;
                        int16_t y;
                };
                Point point;
        };

        Rect rect;
} Tile;

typedef struct {
        Edge edge;
        Edge edge_h;
        Edge edge_v;
        Edge edge_c;
        Tile tile_h;
        Tile tile_v;
        Tile tile_c;
        bool hit_h;
        bool hit_v;
        bool hit_c;
} TileCollision;

/**
 * Get the Tile X/Y from the center of a Rect
 *
 * @param rect coordinate
 * @param tile pointer to the return tile
 *
 * @return Tile representing the tile boundaries, x/y 127 means out of bounds
 */
bool tile_get(Rect* rect, Tile* tile);

/**
 * Get the Tile X/Y from the `edge` of a Rect
 *
 * @param rect coordinate
 * @param edge the edge
 *
 * @return Tile representing the tile boundaries, of the tile on that edge of the rect
 */
bool tile_get_edge(Rect* rect, Edge edge, Tile* tile);

/**
 * Get the Tile located at X/Y in Tile coordinates
 *
 * @param tx tile coordinate
 * @param ty tile coordinate
 * @param tile pointer to return tile
 *
 * @return Tile representing the tile boundaries, x/y 127 means out of bounds
 */
void tile_at(uint8_t tx, uint8_t ty, Tile* tile);

/**
 * Get the Tile containing a point in tilemap pixel coordinates.
 *
 * @param x tilemap pixel x
 * @param y tilemap pixel y
 * @param tile pointer to return tile
 *
 * @return false if point is outside the tilemap
 */
bool tile_get_at_point(int16_t x, int16_t y, Tile* tile);

/**
 * Get candidate tile collisions for a moving Rect.
 *
 * @param rect coordinate
 * @param direction direction rect is moving
 *
 * @param collision pointer to return leading horizontal, vertical, and corner probe candidates
 */
void tile_collide_ex(Rect* rect, Direction* direction, TileCollision* collision);

/**
 * Is `Rect` colliding with a tile?
 *
 * @param rect coordinate
 * @param direction direction rect is moving
 * @param tile pointer to return tile
 *
 * @return the edge of `tile` that `rect` collided with
 */
Edge tile_collide(Rect* rect, Direction* direction, Tile* tile);

#endif

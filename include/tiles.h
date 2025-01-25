#include <stdint.h>
#include "misc.h"

#ifndef TILES_H
#define TILES_H

#define TILE_SIZE           (16U * 16U)
#define TILE_WIDTH          16U
#define TILE_HEIGHT         16U
#define TILE_HALF           8U
#define TILE_QUARTER        4U
#define SCROLL_WIDTH        1280U
#define SCROLL_HEIGHT       640U


typedef struct {
  int8_t x;
  int8_t y;
  Rect rect;
} Tile;

extern uint16_t tilemap_offset_x;
extern uint16_t tilemap_offset_y;

/**
 * Get the Tile X/Y from a Pixel X/Y coordinate
 *
 * @param rect coordinate
 * @param tile pointer to the return tile
 *
 * @return Tile representing the tile boundaries, x/y 127 means out of bounds
 */
bool tile_get(Rect *rect, Tile *tile);
/**
 * Get the Tile located at X/Y in Tile coordinates
  *
 * @param tx tile coordinate
 * @param ty tile coordinate
 * @param tile pointer to return tile
 *
 * @return Tile representing the tile boundaries, x/y 127 means out of bounds
 */
void tile_at(uint8_t tx, uint8_t ty, Tile *tile);

/**
 * Is `Rect` colliding with a tile?
 *
 * @param rect coordinate
 * @param direction direction rect is moving
 * @param tile pointer to return tile
 *
 * @return the edge of `tile` that `rect` collided with
 */
Edge tile_collide(Rect *rect, Direction *direction, Tile *tile);

#endif
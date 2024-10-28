#include <stdint.h>

#ifndef TILES_H
#define TILES_H


/**
 * Get the Tile X/Y from a Pixel X/Y coordinate
 *
 * @param x pixel coordinate
 * @param y pixel coordinate
 */
int16_t tile_get(uint16_t *x, uint16_t *y);


#endif
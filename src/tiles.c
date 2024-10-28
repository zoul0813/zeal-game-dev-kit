#include "types.h"
#include "tiles.h"

static uint16_t tilemap_offset_x;
static uint16_t tilemap_offset_y;


int16_t tile_get(uint16_t *x, uint16_t *y) {
  *x = (*x >> 4) - 1;
  *y = (*y >> 4) - 1;
  return 0;
}
#include <stdint.h>
#include "zgdk/tilemap.h"
#include "zgdk/physics/tileflags.h"
#include <zgdk/utils/grid_rows.h>
#include "tileflags_cache.h"

/* Cached view of the registered ZGDK Tilemap, so lookups skip tilemap_get_xy and its 16-bit multiply */
const uint8_t *_tf_flags = NULL;
const uint8_t *_tf_tiles = NULL;
const Tilemap *_tf_map = NULL;
uint8_t _tf_width, _tf_height;
uint8_t _tf_outside = TF_SOLID;

#define FLAGS   _tf_flags
#define TILES   _tf_tiles
#define MAP     _tf_map
#define WIDTH   _tf_width
#define HEIGHT  _tf_height
#define OUTSIDE _tf_outside
#define ROWS    tileflags_rows

void _tileflags_cache(void) {
    MAP = _TILEMAP;
    TILES = _TILEMAP->tiles;
    WIDTH = (uint8_t)_TILEMAP->rect.w;
    HEIGHT = (uint8_t)(_TILEMAP->rect.h > tileflags_row_capacity ? tileflags_row_capacity : _TILEMAP->rect.h);
    grid_rows_init(ROWS, TILES, WIDTH, HEIGHT);
}

void tileflags_refresh(void) {
    if (_TILEMAP != NULL) _tileflags_cache();
}

void tileflags_register(const uint8_t *table) {
    FLAGS = table;
}

void tileflags_set_outside(uint8_t flags) {
    OUTSIDE = flags;
}

uint8_t tileflags_at_tile(uint8_t tx, uint8_t ty) {
    if (!tileflags_ready()) return OUTSIDE;
    if (tx >= WIDTH || ty >= HEIGHT) return OUTSIDE;
    return FLAGS[ROWS[ty][tx]];
}

uint8_t tileflags_at(int16_t px, int16_t py) {
    if (px < 0 || py < 0) return OUTSIDE;
    return tileflags_at_tile((uint8_t)(px >> 4), (uint8_t)(py >> 4));
}

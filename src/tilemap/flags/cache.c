#include <zgdk/tilemap/flags.h>
#include "cache.h"
#include <zgdk/utils/grid_rows.h>

void _tileflags_cache(void) {
    _tf_map = _TILEMAP;
    _tf_tiles = _TILEMAP->tiles;
    _tf_width = (uint8_t)_TILEMAP->rect.w;
    _tf_height = (uint8_t)(_TILEMAP->rect.h > tileflags_row_capacity ? tileflags_row_capacity : _TILEMAP->rect.h);
    grid_rows_init(tileflags_rows, _tf_tiles, _tf_width, _tf_height);
}

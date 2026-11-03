/* Private tile-flag cache shared by lookups and tilemap collision. */
#include <stdint.h>
#include "zgdk/tilemap.h"
#include <zgdk/inline.h>

#ifndef ZGDK_TILEMAP_FLAGS_CACHE_H
#define ZGDK_TILEMAP_FLAGS_CACHE_H

extern const uint8_t *_tf_flags;
extern const uint8_t *_tf_tiles;
extern const Tilemap *_tf_map;
extern uint8_t _tf_width, _tf_height, _tf_outside;
extern const uint8_t *tileflags_rows[];

void _tileflags_cache(void);

/* true when lookups can use the cache; re-caches after a new tilemap/buffer is registered */
INLINE uint8_t tileflags_ready(void) {
    if (_TILEMAP == NULL || _tf_flags == NULL) return 0;
    if (_TILEMAP != _tf_map || _TILEMAP->tiles != _tf_tiles) _tileflags_cache();
    return 1;
}

#endif

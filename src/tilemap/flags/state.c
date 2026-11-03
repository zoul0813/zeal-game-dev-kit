#include "cache.h"
#include <zgdk/tilemap/flags.h>

/* Cached view of the registered ZGDK Tilemap, so lookups skip tilemap_get_xy and its 16-bit multiply */
const uint8_t *_tf_flags = NULL;
const uint8_t *_tf_tiles = NULL;
const Tilemap *_tf_map = NULL;
uint8_t _tf_width, _tf_height;
uint8_t _tf_outside = TF_SOLID;

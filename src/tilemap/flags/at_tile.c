#include <zgdk/tilemap/flags.h>
#include "cache.h"

uint8_t tileflags_at_tile(uint8_t tx, uint8_t ty) {
    if (!tileflags_ready()) return _tf_outside;
    if (tx >= _tf_width || ty >= _tf_height) return _tf_outside;
    return _tf_flags[tileflags_rows[ty][tx]];
}

#include <zgdk/tilemap/flags.h>
#include "cache.h"

uint8_t tileflags_at(int16_t px, int16_t py) {
    if (px < 0 || py < 0) return _tf_outside;
    return tileflags_at_tile((uint8_t)(px >> 4), (uint8_t)(py >> 4));
}

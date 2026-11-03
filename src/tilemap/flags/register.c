#include <zgdk/tilemap/flags.h>
#include "cache.h"

void tileflags_register(const uint8_t *table) {
    _tf_flags = table;
}

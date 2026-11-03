#include <zgdk/tilemap/flags.h>
#include "cache.h"

void tileflags_refresh(void) {
    if (_TILEMAP != NULL) _tileflags_cache();
}

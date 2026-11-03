/* Each private assembly helper must be extractable without the collision step. */
#include "physics/tilemap/internal.h"
#include <zgdk/tilemap.h>
#include <zgdk/physics/tileflags.h>

TILEFLAGS_DEFINE_CACHE(1);
Tilemap *_TILEMAP;

void main(void) {
#if defined(TEST_EDGES)
    body_t body = {0};
    body_edges(&body);
#elif defined(TEST_ROW)
    body_probe_row(0, 0, 0);
#elif defined(TEST_COLUMN)
    body_probe_column(0, 0, 0);
#endif
}

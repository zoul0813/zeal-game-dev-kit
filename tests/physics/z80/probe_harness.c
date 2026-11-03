/* Direct ABI checks for the production assembly probes and edge helper. */
#include "physics/tilemap/internal.h"
#include <zgdk/tilemap.h>

TILEFLAGS_DEFINE_CACHE(40);
_Static_assert(sizeof(body_t) == 12, "generic body Z80 layout");
Tilemap *_TILEMAP;

uint8_t t_row(int16_t py, int16_t left, int16_t right) {
    return body_probe_row(py, left, right);
}

uint8_t t_col(int16_t px, int16_t top, int16_t bottom) {
    return body_probe_column(px, top, bottom);
}

int16_t TE[4];

void t_edges(body_t *body) {
    body_edges(body);
    TE[0] = body_edge_left;
    TE[1] = body_edge_right;
    TE[2] = body_edge_top;
    TE[3] = body_edge_bottom;
}

void main(void) {
}

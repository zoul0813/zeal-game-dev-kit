#include <zgdk/physics/platformer.h>
/* Z80 harness for the hand-written body code (probes, edges, platformer_move): run by tests/run-physics-z80.sh in a Python Z80 emulator */
#include "physics/tileflags.c"
#include "physics/tilemap.c"
#include "physics/body.c"
#include "physics/platformer/update.c"
#include "physics/platformer/ladder.c"
#include "physics/platformer/move.c"
#include "physics/overlap.c"

TILEFLAGS_DEFINE_CACHE(40);
_Static_assert(sizeof(body_t) == 12, "generic body Z80 layout");
_Static_assert(sizeof(platformer_state_t) == 3, "optional controller Z80 layout");

Tilemap *_TILEMAP;

uint8_t t_row(int16_t py, int16_t left, int16_t right) {
    return probe_row(py, left, right);
}

uint8_t t_col(int16_t px, int16_t top, int16_t bottom) {
    return probe_column(px, top, bottom);
}

Tilemap T_MAP;
platformer_config_t CFG;

void t_map(uint8_t *tiles, uint8_t w, uint8_t h) {
    T_MAP.tiles = tiles;
    T_MAP.rect.w = w;
    T_MAP.rect.h = h;
    _TILEMAP = &T_MAP;
    _tileflags_cache();
}

void t_move(body_t *body) {
    platformer_move(body, &CFG);
}

int16_t TE[4];

void t_edges(body_t *body) {
    edges(body);
    TE[0] = E_LEFT;
    TE[1] = E_RIGHT;
    TE[2] = E_TOP;
    TE[3] = E_BOTTOM;
}

platformer_state_t CONTROL;
uint8_t t_control(body_t *body, uint16_t input) {
    return platformer_update(body, &CFG, &CONTROL, (int8_t)input, !!(input & 256), !!(input & 512));
}

void main(void) {
}

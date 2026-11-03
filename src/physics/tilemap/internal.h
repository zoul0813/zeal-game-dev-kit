#ifndef ZGDK_PHYSICS_TILEMAP_INTERNAL_H
#define ZGDK_PHYSICS_TILEMAP_INTERNAL_H

#include <stddef.h>
#include <zgdk/physics/tilemap.h>

/* Keep the C/assembly layout contract checked by the target compiler. */
_Static_assert(sizeof(tileflags_rows[0]) == 2, "probe asm requires two-byte row pointers");
_Static_assert(offsetof(body_t, x) == 0 && offsetof(body_t, y) == 2, "edges asm offsets");
_Static_assert(offsetof(body_t, w) == 8 && offsetof(body_t, h) == 9, "edges asm offsets");

/* Implemented in body_probe_row.asm, body_probe_column.asm and body_edges.asm
 * using the default SDCC register ABI.
 * Probes require tileflags_ready(); body_edges() fills shared pixel scratch. */
uint8_t body_probe_row(int16_t py, int16_t left, int16_t right);
uint8_t body_probe_column(int16_t px, int16_t top, int16_t bottom);
void body_edges(const body_t *body);
extern int16_t body_edge_left, body_edge_right, body_edge_top, body_edge_bottom;

void body_move_x(body_t *body);
void body_move_y(body_t *body);
void body_ground_check(body_t *body);
void body_touch(body_t *body);
extern uint8_t body_touch_mask;

#endif

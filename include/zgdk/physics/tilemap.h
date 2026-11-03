#ifndef ZGDK_PHYSICS_TILEMAP_H
#define ZGDK_PHYSICS_TILEMAP_H
#include <zgdk/physics/body.h>
#include <zgdk/physics/tileflags.h>

/* Move by velocity and resolve solid/one-way tiles, X then Y; no gravity.
 * Uses the single registered ZGDK Tilemap, independent of camera/VRAM origin.
 * No map/flags: free movement. No swept collision: use <=16 pixels per axis
 * per step, or substep faster motion. Not reentrant (shared probe scratch).
 * BODY_PASS_ONEWAY is preserved; other results and touched are recomputed. */
void body_step(body_t *body);
/* Default 0xff. Zero skips contact/hazard/ladder probes; collision still runs. */
void body_set_touch_mask(uint8_t mask);
#endif

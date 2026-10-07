#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>
#include "zgdk/types.h"

#ifndef SPRITES_H
#define SPRITES_H

/* Register 1..128 eight-byte SDK sprite slots. Initialization tests a one-byte
 * DMA transfer between private RAM bytes and caches whether it completed;
 * firmware versions are not consulted. DMA bus lines must be connected for
 * the probe to succeed. Code, stack and SDK globals reside outside MMU page 0.
 */
gfx_error sprites_register_arena(gfx_sprite* arena, uint8_t size);
gfx_sprite* sprites_register(gfx_sprite sprite);
void sprites_deregister(void);
/* Synchronous upload; DMA for updates of at least eight slots when detected,
 * otherwise Z80 LDIR. Also clears stale hardware slots. Preserves interrupt
 * enable state, MMU mapping and peripheral selection. Caller supplies the
 * usual initialized SDK graphics context and synchronizes with vblank.
 */
gfx_error sprites_render(gfx_context* ctx);

inline void sprite_get_point(gfx_sprite *sprite, Point *point);
inline void sprite_set_point(gfx_sprite *sprite, Point *point);
inline void sprite_set_xy(gfx_sprite *sprite, uint16_t x, uint16_t y);
#endif
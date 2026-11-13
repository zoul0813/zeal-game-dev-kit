#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>
#include "zgdk/types.h"

#ifndef SPRITES_H
#define SPRITES_H

/* size is an element count from 1 through GFX_SPRITES_COUNT, not bytes. */
gfx_error sprites_register_arena(gfx_sprite* arena, uint8_t size);
gfx_sprite* sprites_register(gfx_sprite sprite);
/* Invalidates registered pointers. Render once to hide old hardware slots. */
void sprites_deregister(void);
/* Upload all registered sprites using the ZVB SDK CPU copy, including
 * pending deregistration cleanup. Never probes or uses DMA. */
gfx_error sprites_render(gfx_context* ctx);
/* Use the ZVB SDK CPU copy to upload exactly count sprites from the zero-based arena
 * index start, to the same hardware slots. Other slots are unchanged.
 * Requires ctx and an arena; start + count must not exceed registrations.
 * Zero count is a no-op when start <= registrations. Does not flush pending
 * deregistration cleanup: use sprites_render for that. */
gfx_error sprites_render_range(gfx_context* ctx, uint8_t start, uint8_t count);
/* Explicit DMA variants of the full and range uploads above. Probe DMA lazily
 * on the first nonempty call; return GFX_FAILURE if unavailable (no CPU fallback).
 * DMA transfers do not change CPU page mappings. Validation and cleanup state
 * are shared with the CPU APIs; either full-render API flushes old slots.
 * Failures leave cleanup pending. Empty uploads succeed without probing DMA. */
gfx_error sprites_render_dma(gfx_context* ctx);
gfx_error sprites_render_range_dma(gfx_context* ctx, uint8_t start, uint8_t count);
/* Immediately hide all hardware sprites, without changing the arena or its
 * registrations. No arena is required. Stops on the first SDK error; earlier
 * slots may already be hidden. sprites_render restores registered sprites. */
gfx_error sprites_clear(gfx_context* ctx);

void sprite_get_point(gfx_sprite* sprite, Point* point);
void sprite_set_point(gfx_sprite* sprite, Point* point);
void sprite_set_xy(gfx_sprite* sprite, uint16_t x, uint16_t y);
#endif

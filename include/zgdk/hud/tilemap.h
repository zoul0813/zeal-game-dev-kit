#ifndef ZGDK_HUD_TILEMAP_H
#define ZGDK_HUD_TILEMAP_H

#include <stdint.h>
#include <zvb_gfx.h>

/* Tile-based overlay text using ascii_map(). Coordinates are in tiles;
 * forms without a layer argument use LAYER1. Text must contain printable ASCII
 * only, be at most 40 characters, and fit the destination tilemap. No wrapping.
 * put_s forms read NUL-terminated strings; nput_s forms read exactly len bytes.
 * These void APIs preserve the existing behavior of discarding upload errors. */
void hud_tilemap_put_s(gfx_context *ctx, const char *str, uint8_t x, uint8_t y);
void hud_tilemap_put_s_layer(gfx_context *ctx, const char *str, uint8_t layer, uint8_t x, uint8_t y);
void hud_tilemap_nput_s(gfx_context *ctx, const char *str, uint8_t len, uint8_t x, uint8_t y);
void hud_tilemap_nput_s_layer(gfx_context *ctx, const char *str, uint8_t len, uint8_t layer, uint8_t x, uint8_t y);

#include "zgdk/hud/deprecated.h"
#endif

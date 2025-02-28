#include <stdint.h>
#include <zvb_gfx.h>
#include "zgdk/types.h"

#ifndef TILEMAP_H
#define TILEMAP_H

gfx_error tilemap_fill(gfx_context *ctx, uint8_t layer, uint8_t tile, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
gfx_error tilemap_fill_rect(gfx_context *ctx, uint8_t layer, uint8_t tile, Rect *rect);
gfx_error tilemap_place(gfx_context *ctx, uint8_t layer, uint8_t tile, Point *point);
#endif

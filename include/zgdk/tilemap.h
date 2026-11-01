#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>
#include "zgdk/types.h"

#ifndef TILEMAP_H
#define TILEMAP_H

typedef struct {
    uint8_t *tiles;
    Rect rect;
} Tilemap;

extern Tilemap *_TILEMAP;

/* Draw a tightly packed width*height byte buffer at tile coordinates x,y.
 * Nonzero dimensions must fit one 80x40 hardware layer (layer 0 or 1), without
 * wrapping. On success, populate rect={x,y,width,height}, retain tiles (no copy),
 * and register tilemap. Caller owns the buffer and Tilemap and must keep them
 * valid. The width*height-byte buffer must stay readable while VRAM maps page 0.
 * Invalid parameters return ERR_INVALID_PARAMETER before drawing. An upload
 * failure returns ERR_FAILURE, leaving tilemap and registration unchanged;
 * rows already written to VRAM are not rolled back. No collision cache refresh. */
zos_err_t tilemap_load(gfx_context *ctx, Tilemap *tilemap, uint8_t *tiles,
    uint8_t width, uint8_t height, uint8_t layer, uint8_t x, uint8_t y);

zos_err_t tilemap_register(Tilemap *tilemap, uint8_t *tiles);

int16_t tilemap_get(Point *point);
int16_t tilemap_get_xy(uint8_t x, uint8_t y);

int16_t tilemap_set(Point *point, uint8_t value);
int16_t tilemap_set_xy(uint8_t x, uint8_t y, uint8_t value);

gfx_error tilemap_fill(gfx_context *ctx, uint8_t layer, uint8_t tile, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
gfx_error tilemap_fill_rect(gfx_context *ctx, uint8_t layer, uint8_t tile, Rect *rect);
gfx_error tilemap_place(gfx_context *ctx, uint8_t layer, uint8_t tile, Point *point);
gfx_error tilemap_place_xy(gfx_context *ctx, uint8_t layer, uint8_t tile, uint8_t x, uint8_t y);
#endif

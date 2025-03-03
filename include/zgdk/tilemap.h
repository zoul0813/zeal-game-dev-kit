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

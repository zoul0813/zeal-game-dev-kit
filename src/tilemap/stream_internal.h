#ifndef ZGDK_STREAM_INTERNAL_H
#define ZGDK_STREAM_INTERNAL_H
#include "zgdk/tilemap/stream.h"
uint8_t zgdk_stream_ring_at(uint16_t tile, uint8_t offset, uint8_t period);

/* Archive-internal helpers shared by optional API objects. */
zos_err_t zgdk_stream_init(tilemap_stream_t *s, uint16_t w, uint16_t h,
    uint8_t layer, uint16_t vw, uint16_t vh, int16_t x, int16_t y);
zos_err_t zgdk_stream_write_row(gfx_context *ctx, tilemap_stream_t *s,
    int16_t tx, int16_t ty, uint8_t count);
#endif

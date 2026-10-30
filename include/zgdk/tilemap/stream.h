#ifndef ZGDK_TILEMAP_STREAM_H
#define ZGDK_TILEMAP_STREAM_H
#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>

/* Synchronous, exact-count read of consecutive byte tile indices in one row.
 * The callback owns file/page boundaries, caching and decompression. */
typedef zos_err_t (*tilemap_stream_reader_t)(void *source, uint16_t tile_x,
    uint16_t tile_y, uint8_t *destination, uint8_t count);

typedef struct {
    uint8_t *tiles;
    uint16_t width, height;
    uint16_t view_w, view_h;
    uint8_t layer;
    int16_t tx, ty;
    uint8_t ring_x, ring_y;
    uint8_t ready, dirty;
    uint8_t cols, rows;
    tilemap_stream_reader_t read;
    void *source;
    uint8_t *scratch;
} tilemap_stream_t;

/* Nonzero map dimensions <=32767 tiles. RAM must be a complete, addressable
 * buffer of width*height <=65535 bytes. Caller owns all storage and its lifetime.
 * View coverage is ceil((pixels+15)/16), at most 80 columns and 40 rows.
 * Pixel coordinates are signed top-left positions; cells outside the map are
 * left untouched. Init draws immediately; rendering failure leaves dirty set. */
zos_err_t tilemap_stream_init_ram(gfx_context *ctx, tilemap_stream_t *stream,
    uint8_t *tiles, uint16_t width, uint16_t height, uint8_t layer,
    uint16_t view_w, uint16_t view_h, int16_t x, int16_t y);
/* scratch_size must cover one viewport row (the coverage above). */
zos_err_t tilemap_stream_init_reader(gfx_context *ctx, tilemap_stream_t *stream,
    tilemap_stream_reader_t read, void *source, uint8_t *scratch, uint8_t scratch_size,
    uint16_t width, uint16_t height, uint8_t layer,
    uint16_t view_w, uint16_t view_h, int16_t x, int16_t y);
zos_err_t tilemap_stream_update(gfx_context *ctx, tilemap_stream_t *stream, int16_t x, int16_t y);
zos_err_t tilemap_stream_redraw(gfx_context *ctx, tilemap_stream_t *stream);
zos_err_t tilemap_stream_set_tile(gfx_context *ctx, tilemap_stream_t *stream,
    uint16_t tx, uint16_t ty, uint8_t tile);
/* Positive tile offsets (zero allowed), bounded by map dimensions and by the
 * resulting signed loaded-window coordinates. Rebase changes only ring state.
 * Shift compacts RAM only, leaving vacated bytes and ring state untouched.
 * Reader shifts/edits return ERR_NOT_SUPPORTED. Invalid calls are atomic. */
zos_err_t tilemap_stream_rebase(tilemap_stream_t *stream, uint16_t dtx, uint16_t dty);
zos_err_t tilemap_stream_shift_buffer(tilemap_stream_t *stream, uint16_t dtx, uint16_t dty);
#endif

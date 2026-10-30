#include "stream_internal.h"

zos_err_t tilemap_stream_init_reader(gfx_context *ctx, tilemap_stream_t *s,
    tilemap_stream_reader_t read, void *source, uint8_t *scratch, uint8_t size,
    uint16_t w, uint16_t h, uint8_t layer,
    uint16_t vw, uint16_t vh, int16_t x, int16_t y) {
    zos_err_t err;
    if (!read || !scratch || vw > 1265 || size < (vw + 30) / 16) return ERR_INVALID_PARAMETER;
    err = zgdk_stream_init(s, w, h, layer, vw, vh, x, y);
    if (err != ERR_SUCCESS) return err;
    s->tiles = 0; s->read = read; s->source = source; s->scratch = scratch;
    return tilemap_stream_redraw(ctx, s);
}

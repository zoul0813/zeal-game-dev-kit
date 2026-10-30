#include "stream_internal.h"

zos_err_t tilemap_stream_init_ram(gfx_context *ctx, tilemap_stream_t *s,
    uint8_t *tiles, uint16_t w, uint16_t h, uint8_t layer,
    uint16_t vw, uint16_t vh, int16_t x, int16_t y) {
    zos_err_t err;
    if (!tiles || !w || h > 65535U / w) return ERR_INVALID_PARAMETER;
    err = zgdk_stream_init(s, w, h, layer, vw, vh, x, y);
    if (err != ERR_SUCCESS) return err;
    s->tiles = tiles; s->read = 0; s->source = 0; s->scratch = 0;
    return tilemap_stream_redraw(ctx, s);
}

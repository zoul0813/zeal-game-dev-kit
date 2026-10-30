#include "stream_internal.h"

zos_err_t tilemap_stream_set_tile(gfx_context *ctx, tilemap_stream_t *s,
    uint16_t tx, uint16_t ty, uint8_t tile) {
    if (!s || !s->ready) return ERR_INVALID_PARAMETER;
    if (s->read) return ERR_NOT_SUPPORTED;
    if (tx >= s->width || ty >= s->height) return ERR_INVALID_PARAMETER;
    s->tiles[(uint16_t)(ty * s->width + tx)] = tile;
    /* Map coordinates and loaded positions differ by at most 65534. The
     * unsigned distance also rejects cells left/above the resident window. */
    if ((uint16_t)(tx - (uint16_t)s->tx) >= s->cols ||
        (uint16_t)(ty - (uint16_t)s->ty) >= s->rows) return ERR_SUCCESS;
    {
        zos_err_t err = zgdk_stream_write_row(ctx, s, tx, ty, 1);
        if (err != ERR_SUCCESS) s->dirty = 1;
        return err;
    }
}

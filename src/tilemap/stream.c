#include "zgdk/tilemap/stream.h"
#include "stream_internal.h"

#define RING_W 80
#define RING_H 40

static int16_t tile_at(int16_t pixel) {
    /* C division truncates toward zero; use floor for negative coordinates. */
    return pixel >= 0 ? pixel / 16 : -1 - (int16_t)(-(int32_t)pixel - 1) / 16;
}
static uint8_t ring_at(int16_t tile, uint8_t offset, uint8_t period) {
    int16_t r = tile % period;
    if (r < 0) r += period;
    r += offset;
    if (r >= period) r -= period;
    return (uint8_t)r;
}
static zos_err_t finish(tilemap_stream_t *s, zos_err_t err) {
    if (err != ERR_SUCCESS) s->dirty = 1;
    return err;
}
static zos_err_t fetch(tilemap_stream_t *s, uint16_t x, uint16_t y,
                        uint8_t count, uint8_t **data) {
    if (!s->read) {
        *data = s->tiles + (uint16_t)(y * s->width + x);
        return ERR_SUCCESS;
    }
    *data = s->scratch;
    return s->read(s->source, x, y, s->scratch, count);
}
zos_err_t zgdk_stream_write_row(gfx_context *ctx, tilemap_stream_t *s,
                            int16_t tx, int16_t ty, uint8_t count) {
    int16_t end = tx + count;
    uint8_t *src, vx, vy, first;
    zos_err_t err;
    if (ty < 0 || ty >= (int16_t)s->height || end <= 0 || tx >= (int16_t)s->width) return ERR_SUCCESS;
    if (tx < 0) tx = 0;
    if (end > s->width) end = s->width;
    count = (uint8_t)(end - tx);
    err = fetch(s, tx, ty, count, &src);
    if (err != ERR_SUCCESS) return err;
    vx = ring_at(tx, s->ring_x, RING_W); vy = ring_at(ty, s->ring_y, RING_H);
    first = count > RING_W - vx ? RING_W - vx : count;
    if (gfx_tilemap_load(ctx, src, first, s->layer, vx, vy) != GFX_SUCCESS) return ERR_FAILURE;
    if (first < count && gfx_tilemap_load(ctx, src + first, count - first, s->layer, 0, vy) != GFX_SUCCESS)
        return ERR_FAILURE;
    return ERR_SUCCESS;
}
static zos_err_t write_col(gfx_context *ctx, tilemap_stream_t *s,
                            int16_t tx, int16_t ty, uint8_t count) {
    int16_t end = ty + count;
    uint8_t *src, vx, vy;
    zos_err_t err;
    if (tx < 0 || tx >= (int16_t)s->width || end <= 0 || ty >= (int16_t)s->height) return ERR_SUCCESS;
    if (ty < 0) ty = 0;
    if (end > s->height) end = s->height;
    vx = ring_at(tx, s->ring_x, RING_W); vy = ring_at(ty, s->ring_y, RING_H);
    while (ty < end) {
        err = fetch(s, tx, ty, 1, &src);
        if (err != ERR_SUCCESS) return err;
        if (gfx_tilemap_place(ctx, *src, s->layer, vx, vy) != GFX_SUCCESS) return ERR_FAILURE;
        ty++;
        if (++vy == RING_H) vy = 0;
    }
    return ERR_SUCCESS;
}
zos_err_t tilemap_stream_redraw(gfx_context *ctx, tilemap_stream_t *s) {
    uint8_t i;
    zos_err_t err;
    if (!s || !s->ready) return ERR_INVALID_PARAMETER;
    s->dirty = 1;
    for (i = 0; i < s->rows; i++) {
        /* Loaded tiles originate in int16 pixel coordinates and rebase only
         * subtracts, so this sum cannot exceed INT16_MAX. */
        err = zgdk_stream_write_row(ctx, s, s->tx, s->ty + i, s->cols);
        if (err != ERR_SUCCESS) return err;
    }
    s->dirty = 0;
    return ERR_SUCCESS;
}
zos_err_t zgdk_stream_init(tilemap_stream_t *s, uint16_t w, uint16_t h,
                       uint8_t layer, uint16_t vw, uint16_t vh, int16_t x, int16_t y) {
    if (!s || !w || !h || w > 32767 || h > 32767 || layer > 1 || !vw || !vh ||
        vw > 1265 || vh > 625)
        return ERR_INVALID_PARAMETER;
    s->width = w; s->height = h; s->layer = layer;
    s->view_w = vw; s->view_h = vh;
    s->cols = (vw + 30) / 16; s->rows = (vh + 30) / 16;
    s->tx = tile_at(x); s->ty = tile_at(y);
    s->ring_x = s->ring_y = 0;
    s->ready = s->dirty = 1;
    return ERR_SUCCESS;
}
zos_err_t tilemap_stream_init_ram(gfx_context *ctx, tilemap_stream_t *s,
    uint8_t *tiles, uint16_t w, uint16_t h, uint8_t layer,
    uint16_t vw, uint16_t vh, int16_t x, int16_t y) {
    zos_err_t err;
    if (!tiles || (uint32_t)w * h > 65535UL) return ERR_INVALID_PARAMETER;
    err = zgdk_stream_init(s, w, h, layer, vw, vh, x, y);
    if (err != ERR_SUCCESS) return err;
    s->tiles = tiles; s->read = 0; s->source = 0; s->scratch = 0;
    return tilemap_stream_redraw(ctx, s);
}
zos_err_t tilemap_stream_update(gfx_context *ctx, tilemap_stream_t *s, int16_t x, int16_t y) {
    int16_t tx = tile_at(x), ty = tile_at(y), dx, dy;
    zos_err_t err;
    if (!s || !s->ready) return ERR_INVALID_PARAMETER;
    /* Pixel-derived tile coordinates are -2048..2047. Compare their neighbors
     * before subtracting an old, possibly rebased window near INT16_MIN. */
    if (s->dirty || tx - 1 > s->tx || tx + 1 < s->tx ||
        ty - 1 > s->ty || ty + 1 < s->ty) {
        s->tx = tx; s->ty = ty;
        return tilemap_stream_redraw(ctx, s);
    }
    dx = tx - s->tx; dy = ty - s->ty;
    if (dx) {
        s->tx = tx;
        err = write_col(ctx, s, dx > 0 ? tx + s->cols - 1 : tx, s->ty, s->rows);
        if (err != ERR_SUCCESS) return finish(s, err);
    }
    if (dy) {
        s->ty = ty;
        err = zgdk_stream_write_row(ctx, s, s->tx, dy > 0 ? ty + s->rows - 1 : ty, s->cols);
        if (err != ERR_SUCCESS) return finish(s, err);
    }
    return ERR_SUCCESS;
}

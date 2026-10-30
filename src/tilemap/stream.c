#include "zgdk/tilemap/stream.h"
#include "stream_internal.h"

#define RING_W 80
#define RING_H 40

/* Floor for negative coordinates too: SDCC (and the host compilers) shift
 * signed values arithmetically. */
#define tile_at(pixel) ((int16_t)((pixel) >> 4))
/* Already clipped source coordinates and their ring positions. */
static zos_err_t write_row_at(gfx_context *ctx, tilemap_stream_t *s,
    uint16_t tx, uint16_t ty, uint8_t count, uint8_t vx, uint8_t vy) {
    uint8_t *src, first;
    zos_err_t err;
    if (s->read) {
        src = s->scratch;
        err = s->read(s->source, tx, ty, src, count);
        if (err != ERR_SUCCESS) return err;
    } else {
        src = s->tiles + (uint16_t)((uint16_t)ty * s->width + tx);
    }
    first = RING_W - vx;
    if (first > count) first = count;
    if (gfx_tilemap_load(ctx, src, first, s->layer, vx, vy) != GFX_SUCCESS) return ERR_FAILURE;
    count -= first;
    if (count && gfx_tilemap_load(ctx, src + first, count, s->layer, 0, vy) != GFX_SUCCESS)
        return ERR_FAILURE;
    return ERR_SUCCESS;
}
zos_err_t zgdk_stream_write_row(gfx_context *ctx, tilemap_stream_t *s,
                            int16_t tx, int16_t ty, uint8_t count) {
    int16_t end = tx + count;
    if ((uint16_t)ty >= s->height) return ERR_SUCCESS;
    if (tx < 0) tx = 0;
    if (end > (int16_t)s->width) end = s->width;
    if (end <= tx) return ERR_SUCCESS;
    return write_row_at(ctx, s, tx, ty, (uint8_t)(end - tx),
        zgdk_stream_ring_at(tx, s->ring_x, RING_W),
        zgdk_stream_ring_at(ty, s->ring_y, RING_H));
}
static zos_err_t write_col(gfx_context *ctx, tilemap_stream_t *s,
                            int16_t tx, int16_t ty, uint8_t count) {
    int16_t end = ty + count;
    uint16_t step;
    uint8_t *src, vx, vy;
    zos_err_t err;
    if ((uint16_t)tx >= s->width) return ERR_SUCCESS;
    if (ty < 0) ty = 0;
    if (end > (int16_t)s->height) end = s->height;
    if (end <= ty) return ERR_SUCCESS;
    count = (uint8_t)(end - ty);
    vx = zgdk_stream_ring_at(tx, s->ring_x, RING_W); vy = zgdk_stream_ring_at(ty, s->ring_y, RING_H);
    /* RAM: one multiply, then step down the column. A reader refills scratch per tile. */
    if (s->read) { src = s->scratch; step = 0; }
    else { step = s->width; src = s->tiles + (uint16_t)((uint16_t)ty * step + tx); }
    for (;;) {
        if (s->read) {
            err = s->read(s->source, tx, ty, src, 1);
            if (err != ERR_SUCCESS) return err;
        }
        if (gfx_tilemap_place(ctx, *src, s->layer, vx, vy) != GFX_SUCCESS) return ERR_FAILURE;
        /* stop before stepping past the last row: src stays inside the map */
        if (!--count) return ERR_SUCCESS;
        ty++; src += step;
        if (++vy == RING_H) vy = 0;
    }
}
zos_err_t tilemap_stream_redraw(gfx_context *ctx, tilemap_stream_t *s) {
    int16_t tx, ty, end_x, end_y;
    uint8_t vx, vy, count;
    zos_err_t err;
    if (!s || !s->ready) return ERR_INVALID_PARAMETER;
    s->dirty = 1;
    /* Clip once, then advance the ring row without repeating its reduction.
     * Pixel-derived coordinates and their coverage stay below INT16_MAX;
     * rebasing only subtracts from them. */
    tx = s->tx; ty = s->ty;
    end_x = tx + s->cols; end_y = ty + s->rows;
    if (tx < 0) tx = 0;
    if (ty < 0) ty = 0;
    if (end_x > (int16_t)s->width) end_x = s->width;
    if (end_y > (int16_t)s->height) end_y = s->height;
    if (end_x > tx && end_y > ty) {
        count = (uint8_t)(end_x - tx);
        vx = zgdk_stream_ring_at(tx, s->ring_x, RING_W);
        vy = zgdk_stream_ring_at(ty, s->ring_y, RING_H);
        do {
            err = write_row_at(ctx, s, tx, ty, count, vx, vy);
            if (err != ERR_SUCCESS) return err;
            if (++vy == RING_H) vy = 0;
        } while (++ty < end_y);
    }
    s->dirty = 0;
    return ERR_SUCCESS;
}
zos_err_t zgdk_stream_init(tilemap_stream_t *s, uint16_t w, uint16_t h,
                       uint8_t layer, uint16_t vw, uint16_t vh, int16_t x, int16_t y) {
    /* signed casts reject 0 and anything above 32767 in one compare */
    if (!s || (int16_t)w <= 0 || (int16_t)h <= 0 || layer > 1 ||
        (uint16_t)(vw - 1) >= 1265 || (uint16_t)(vh - 1) >= 625)
        return ERR_INVALID_PARAMETER;
    s->width = w; s->height = h; s->layer = layer;
    s->view_w = vw; s->view_h = vh;
    s->cols = (uint8_t)((vw + 30) >> 4); s->rows = (uint8_t)((vh + 30) >> 4);
    s->tx = tile_at(x); s->ty = tile_at(y);
    s->ring_x = s->ring_y = 0;
    s->ready = s->dirty = 1;
    return ERR_SUCCESS;
}
zos_err_t tilemap_stream_update(gfx_context *ctx, tilemap_stream_t *s, int16_t x, int16_t y) {
    int16_t tx = tile_at(x), ty = tile_at(y);
    uint16_t dx, dy;
    zos_err_t err = ERR_SUCCESS;
    if (!s || !s->ready) return ERR_INVALID_PARAMETER;
    /* Pixel-derived tiles are -2048..2047 and a rebased window never drops below
     * INT16_MIN, so the true difference spans under 64K: as a 16-bit wrapping
     * value, only steps of -1, 0 and +1 land on 0xFFFF, 0 and 1. */
    dx = (uint16_t)tx - (uint16_t)s->tx;
    dy = (uint16_t)ty - (uint16_t)s->ty;
    if (s->dirty || (uint16_t)(dx + 1) > 2 || (uint16_t)(dy + 1) > 2) {
        s->tx = tx; s->ty = ty;
        return tilemap_stream_redraw(ctx, s);
    }
    if (dx) {
        s->tx = tx;
        if (dx == 1) tx += s->cols - 1;
        err = write_col(ctx, s, tx, s->ty, s->rows);
    }
    if (dy && err == ERR_SUCCESS) {
        s->ty = ty;
        if (dy == 1) ty += s->rows - 1;
        err = zgdk_stream_write_row(ctx, s, s->tx, ty, s->cols);
    }
    if (err != ERR_SUCCESS) s->dirty = 1;
    return err;
}

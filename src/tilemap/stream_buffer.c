#include "zgdk/tilemap/stream.h"
#include <string.h>
#define RING_W 80
#define RING_H 40

/* Flipping the sign bit gives the unsigned distance to INT16_MIN. */
zos_err_t tilemap_stream_rebase(tilemap_stream_t *s, uint16_t dx, uint16_t dy) {
    if (!s || !s->ready || dx >= s->width || dy >= s->height ||
        dx > ((uint16_t)s->tx ^ 0x8000U) || dy > ((uint16_t)s->ty ^ 0x8000U))
        return ERR_INVALID_PARAMETER;
    s->tx -= dx; s->ty -= dy;
    s->ring_x = (s->ring_x + dx) % RING_W;
    s->ring_y = (s->ring_y + dy) % RING_H;
    return ERR_SUCCESS;
}
zos_err_t tilemap_stream_shift_buffer(tilemap_stream_t *s, uint16_t dx, uint16_t dy) {
    uint16_t rows, width, count;
    uint8_t *dst, *src;
    if (!s || !s->ready) return ERR_INVALID_PARAMETER;
    if (s->read) return ERR_NOT_SUPPORTED;
    if (dx >= s->width || dy >= s->height) return ERR_INVALID_PARAMETER;
    width = s->width; rows = s->height - dy; count = width - dx;
    dst = s->tiles; src = dst + (uint16_t)(dy * width) + dx;
    if (!dx) {
        if (dy) memmove(dst, src, (uint16_t)(rows * width));
    } else for (;;) {
        memmove(dst, src, count);
        if (!--rows) break;
        dst += width; src += width;
    }
    return ERR_SUCCESS;
}

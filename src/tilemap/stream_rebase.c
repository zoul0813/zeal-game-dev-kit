#include "stream_internal.h"

/* Flipping the sign bit gives the unsigned distance to INT16_MIN. */
zos_err_t tilemap_stream_rebase(tilemap_stream_t *s, uint16_t dx, uint16_t dy) {
    if (!s || !s->ready || dx >= s->width || dy >= s->height ||
        dx > ((uint16_t)s->tx ^ 0x8000U) || dy > ((uint16_t)s->ty ^ 0x8000U))
        return ERR_INVALID_PARAMETER;
    s->tx -= dx; s->ty -= dy;
    s->ring_x = zgdk_stream_ring_at(dx, s->ring_x, 80);
    s->ring_y = zgdk_stream_ring_at(dy, s->ring_y, 40);
    return ERR_SUCCESS;
}

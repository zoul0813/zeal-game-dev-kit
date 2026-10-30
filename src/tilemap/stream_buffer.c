#include "zgdk/tilemap/stream.h"
#include <core.h>

zos_err_t tilemap_stream_shift_buffer(tilemap_stream_t *s, uint16_t dx, uint16_t dy) {
    uint16_t rows, width, count;
    uint8_t *dst, *src;
    if (!s || !s->ready) return ERR_INVALID_PARAMETER;
    if (s->read) return ERR_NOT_SUPPORTED;
    if (dx >= s->width || dy >= s->height) return ERR_INVALID_PARAMETER;
    width = s->width; rows = s->height - dy; count = width - dx;
    dst = s->tiles; src = dst + (uint16_t)(dy * width) + dx;
    /* dst is below src, and core's mem_cpy copies forward (LDIR), so the overlap is safe */
    if (!dx) {
        if (dy) mem_cpy(dst, src, (uint16_t)(rows * width));
    } else for (;;) {
        mem_cpy(dst, src, count);
        if (!--rows) break;
        dst += width; src += width;
    }
    return ERR_SUCCESS;
}

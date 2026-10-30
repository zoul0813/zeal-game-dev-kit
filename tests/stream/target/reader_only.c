/* Archive extraction proof: reader streams and rebasing need no RAM helpers. */
#include "zgdk/tilemap/stream.h"

static uint8_t scratch[2];
static tilemap_stream_t stream;

static zos_err_t terrain(void *source, uint16_t x, uint16_t y,
                         uint8_t *dst, uint8_t count) {
    (void)source;
    while (count--) *dst++ = (uint8_t)(x++ + y);
    return ERR_SUCCESS;
}
gfx_error gfx_tilemap_load(gfx_context *ctx, void *src, uint8_t count,
                          uint8_t layer, uint8_t x, uint8_t y) {
    (void)ctx; (void)src; (void)count; (void)layer; (void)x; (void)y;
    return GFX_SUCCESS;
}
gfx_error gfx_tilemap_place(gfx_context *ctx, uint8_t tile,
                           uint8_t layer, uint8_t x, uint8_t y) {
    (void)ctx; (void)tile; (void)layer; (void)x; (void)y;
    return GFX_SUCCESS;
}
int main(void) {
    if (tilemap_stream_init_reader(0, &stream, terrain, 0, scratch, sizeof scratch,
            32767, 32767, 0, 16, 16, 16, 16) != ERR_SUCCESS) return 1;
    if (tilemap_stream_rebase(&stream, 32766, 32766) != ERR_SUCCESS) return 2;
    if (stream.ring_x != 46 || stream.ring_y != 6) return 3;
    return tilemap_stream_update(0, &stream, 0, 0) != ERR_SUCCESS;
}

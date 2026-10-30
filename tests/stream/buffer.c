/* Link the stream on its own: no camera, body, entity, or collision registry. */
#include <assert.h>
#include <stdio.h>
#include "zgdk/tilemap/stream.h"

static uint8_t vram[40][80], tiles[12 * 24];
gfx_error gfx_tilemap_load(gfx_context *ctx, void *src, uint8_t count,
                          uint8_t layer, uint8_t x, uint8_t y) {
    uint8_t i;
    (void)ctx;
    assert(layer == 0 && x + count <= 80 && y < 40 && count);
    for (i = 0; i < count; i++) vram[y][x+i] = ((uint8_t *)src)[i];
    return GFX_SUCCESS;
}
gfx_error gfx_tilemap_place(gfx_context *ctx, uint8_t tile, uint8_t layer, uint8_t x, uint8_t y) {
    return gfx_tilemap_load(ctx, &tile, 1, layer, x, y);
}
static uint8_t terrain(unsigned row, unsigned col) { return (uint8_t)(row*3 + col*5); }
int main(void) {
    tilemap_stream_t stream;
    int16_t px = 0, py = 0;
    unsigned base = 0, step;
    int x, y;
    for (y = 0; y < 24; y++) for (x = 0; x < 12; x++) tiles[y*12+x] = terrain(y,x);
    assert(tilemap_stream_init_ram(0,&stream,tiles,12,24,0,100,100,px,py)==ERR_SUCCESS);
    for (step = 0; step < 20000; step++) {
        px += 2; py += 2;
        if (px >= 64 && py >= 64) {
            assert(tilemap_stream_shift_buffer(&stream,1,1)==ERR_SUCCESS);
            assert(tilemap_stream_rebase(&stream,1,1)==ERR_SUCCESS);
            px -= 16; py -= 16; base++;
            for (y = 0; y < 24; y++) for (x = 0; x < 12; x++)
                if (x == 11 || y == 23) tiles[y*12+x] = terrain(base+y,base+x);
        }
        assert(tilemap_stream_update(0,&stream,px,py)==ERR_SUCCESS);
        for (y = py/16; y <= (py+99)/16; y++) for (x = px/16; x <= (px+99)/16; x++)
            assert(vram[(y+stream.ring_y)%40][(x+stream.ring_x)%80]==terrain(base+y,base+x));
    }
    assert(base > 1000);
    puts("stream-only: 20,000 coordinated updates passed without camera/entity dependencies");
    return 0;
}

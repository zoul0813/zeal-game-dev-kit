/* Runs against the real 16-bit SDCC ABI, with software Video Board stubs.
 * Break at test_done and inspect test_result (1=pass) and test_failure. */
#include <stdint.h>
#include <string.h>
#include "zgdk/camera.h"
#include "zgdk/tilemap/stream.h"

volatile uint8_t test_result;
volatile uint32_t test_failure;
static uint32_t check_number;
static uint8_t vram[40][80], tiles[16], scratch[2];
static uint16_t scroll_x, scroll_y;
static uint8_t read_fail, gfx_fail;

static void check(uint8_t success) {
    check_number++;
    if (!success && !test_failure) test_failure = check_number;
}
void tilemap_scroll(uint8_t layer, uint16_t x, uint16_t y) {
    check(layer < 2 && x < 1280 && y < 640);
    scroll_x = x; scroll_y = y;
}
gfx_error gfx_tilemap_load(gfx_context *ctx, void *data, uint8_t count,
                          uint8_t layer, uint8_t x, uint8_t y) {
    (void)ctx;
    check(layer < 2 && x + count <= 80 && y < 40 && count);
    if (gfx_fail) { gfx_fail = 0; return (gfx_error)17; }
    memcpy(&vram[y][x], data, count);
    return GFX_SUCCESS;
}
gfx_error gfx_tilemap_place(gfx_context *ctx, uint8_t tile, uint8_t layer, uint8_t x, uint8_t y) {
    return gfx_tilemap_load(ctx, &tile, 1, layer, x, y);
}
static zos_err_t read_tiles(void *source, uint16_t x, uint16_t y, uint8_t *dst, uint8_t count) {
    (void)source;
    check(x + count <= 4 && y < 4 && count);
    if (read_fail) { read_fail = 0; return ERR_INVALID_OFFSET; }
    memcpy(dst, tiles + y * 4 + x, count);
    return ERR_SUCCESS;
}
void test_done(void) {
    while (1) {}
}
int main(void) {
    tilemap_stream_t s;
    camera_t c;
    uint16_t i;
    uint8_t shift;
    uint32_t absolute_x = 0, absolute_y = 0;
    for (i = 0; i < 16; i++) tiles[i] = i + 1;
    memset(vram, 0xCC, sizeof vram);
    check(tilemap_stream_init_ram(0, &s, tiles, 4, 4, 0, 16, 16, -1, -1) == ERR_SUCCESS);
    check(s.tx == -1 && s.ty == -1 && vram[0][0] == 1 && vram[39][79] == 0xCC);
    check(tilemap_stream_update(0, &s, 15, 15) == ERR_SUCCESS);
    check(vram[0][0] == 1 && vram[0][1] == 2 && vram[1][0] == 5 && vram[1][1] == 6);
    check(tilemap_stream_set_tile(0, &s, 1, 1, 42) == ERR_SUCCESS && vram[1][1] == 42);
    gfx_fail = 1;
    check(tilemap_stream_set_tile(0, &s, 1, 1, 43) == ERR_FAILURE && s.dirty);
    check(tilemap_stream_update(0, &s, 15, 15) == ERR_SUCCESS && !s.dirty && vram[1][1] == 43);
    check(tilemap_stream_init_ram(0, &s, tiles, 256, 256, 0, 16, 16, 0, 0) == ERR_INVALID_PARAMETER);
    check(tilemap_stream_init_reader(0, &s, read_tiles, 0, scratch, 2, 4, 4, 0, 16, 16, -1, -1) == ERR_SUCCESS);
    check(vram[0][0] == 1);
    read_fail = 1;
    check(tilemap_stream_update(0, &s, 16, 16) == ERR_INVALID_OFFSET && s.dirty);
    check(tilemap_stream_update(0, &s, 16, 16) == ERR_SUCCESS && !s.dirty && vram[1][1] == 43);
    check(tilemap_stream_update(0, &s, -32768, -32768) == ERR_SUCCESS && s.tx == -2048 && s.ty == -2048);
    check(tilemap_stream_update(0, &s, 32767, 32767) == ERR_SUCCESS && s.tx == 2047 && s.ty == 2047);
    for (i = 0; i < 16; i++) tiles[i] = i + 1;
    check(tilemap_stream_init_ram(0, &s, tiles, 4, 4, 0, 16, 16, 16, 16) == ERR_SUCCESS);
    check(tilemap_stream_shift_buffer(&s, 1, 1) == ERR_SUCCESS);
    check(tiles[0] == 6 && tiles[1] == 7 && tiles[3] == 4 && tiles[12] == 13 && tiles[15] == 16);
    check(tilemap_stream_rebase(&s, 1, 1) == ERR_SUCCESS && s.tx == 0 && s.ty == 0 && s.ring_x == 1 && s.ring_y == 1);
    check(tilemap_stream_update(0, &s, 0, 0) == ERR_SUCCESS && vram[1][1] == tiles[0] && vram[2][2] == tiles[5]);
    check(camera_init_view(&c, 65535U, 65535U, 65535U, 65535U) == ERR_INVALID_PARAMETER);
    check(camera_init_view(&c, CAMERA_MAX_SIZE + 1, 100, 320, 240) == ERR_INVALID_PARAMETER);
    check(camera_init_view(&c, CAMERA_MAX_SIZE, CAMERA_MAX_SIZE, CAMERA_MAX_SIZE, CAMERA_MAX_SIZE) == ERR_SUCCESS);
    check(c.max_x == 0 && c.dz_left == 5461 && c.dz_top == 6143);
    /* the ends of the documented ranges, under 16-bit int promotion */
    check(camera_init_view(&c, CAMERA_MAX_SIZE, CAMERA_MAX_SIZE, 1, 1) == ERR_SUCCESS);
    camera_set(&c, CAMERA_MAX_SIZE, CAMERA_MAX_SIZE);
    check(c.x == CAMERA_MAX_SIZE - 1 && c.y == CAMERA_MAX_SIZE - 1);
    c.dz_left = c.dz_top = 0; c.max_step = 255; c.lead = 255;
    camera_follow_target(&c, CAMERA_MAX_SIZE - 1, CAMERA_MAX_SIZE - 1, CAMERA_MAX_TARGET, CAMERA_MAX_TARGET, 1);
    check(c.x == CAMERA_MAX_SIZE - 1 && c.y == CAMERA_MAX_SIZE - 1);
    check(camera_init_view(&c, CAMERA_MAX_SIZE, CAMERA_MAX_SIZE, CAMERA_MAX_SIZE / 2, CAMERA_MAX_SIZE / 2) == ERR_SUCCESS);
    c.lead = 255; c.max_step = 0;
    camera_set(&c, 4000, 4000);
    camera_follow_target(&c, CAMERA_TARGET_MIN, CAMERA_TARGET_MIN, CAMERA_MAX_TARGET, CAMERA_MAX_TARGET, -1);
    check(c.x == 0 && c.y == 0);
    camera_follow_target(&c, CAMERA_MAX_SIZE - 1, CAMERA_MAX_SIZE - 1, CAMERA_MAX_TARGET, CAMERA_MAX_TARGET, 1);
    check(c.x == c.max_x && c.y == c.max_y);
    c.x = c.y = 32767;
    check(camera_rebase(&c, 65535U, 65535U) == ERR_INVALID_PARAMETER);
    check(camera_rebase(&c, 32767, 32767) == ERR_SUCCESS && c.x == 0 && c.y == 0);
    check(camera_rebase(&c, 1, 0) == ERR_INVALID_PARAMETER);
    check(camera_init_view(&c, 2000, 1000, 320, 240) == ERR_SUCCESS);
    c.origin_x = (1280U << CAMERA_MAX_SHIFT) - 1; c.origin_y = (640U << CAMERA_MAX_SHIFT) - 1;
    c.x = 32767; c.y = 32767;
    for (shift = 0; shift <= CAMERA_MAX_SHIFT; shift++) {
        check(camera_scroll_layer(&c, 0, shift, shift) == ERR_SUCCESS);
        check(scroll_x == ((((1280UL << CAMERA_MAX_SHIFT) - 1 + 32767) >> shift) % 1280));
        check(scroll_y == ((((640UL << CAMERA_MAX_SHIFT) - 1 + 32767) >> shift) % 640));
    }
    check(camera_init_view(&c, 2000, 1000, 320, 240) == ERR_SUCCESS);
    for (i = 0; i < 43000U; i++) {
        camera_set(&c, 1000, 600);
        absolute_x += 1000; absolute_y += 600;
        check(camera_rebase(&c, 1000, 600) == ERR_SUCCESS);
        if (i % 97 == 0) for (shift = 0; shift <= CAMERA_MAX_SHIFT; shift++) {
            check(camera_scroll_layer(&c, 0, shift, CAMERA_MAX_SHIFT - shift) == ERR_SUCCESS);
            check(scroll_x == ((absolute_x >> shift) % 1280));
            check(scroll_y == ((absolute_y >> (CAMERA_MAX_SHIFT - shift)) % 640));
        }
    }
    test_result = test_failure ? 2 : 1;
    test_done();
    return 0;
}

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <zgdk/hud.h>
#include <zgdk/ascii.h>

static uint8_t tiles[40], size, layer, x, y;
static unsigned calls;
static gfx_context context;
gfx_error gfx_tilemap_load(gfx_context *ctx, void *data, uint8_t count,
                           uint8_t destination, uint8_t dx, uint8_t dy) {
    assert(ctx == &context && count <= sizeof(tiles));
    ++calls;
    size = count; layer = destination; x = dx; y = dy;
    memcpy(tiles, data, count);
    return count ? GFX_SUCCESS : GFX_INVALID_ARG;
}

static void expect(uint8_t expected_layer, uint8_t expected_size) {
    static const uint8_t abc[] = {160, 161, 162};
    assert(layer == expected_layer && size == expected_size && x == 2 && y == 3);
    assert(!memcmp(tiles, abc, expected_size));
}

int main(void) {
    const char bytes[3] = {'A', 'B', 'C'};
    char full[40];
    ascii_map('A', 3, 160);
    hud_tilemap_put_s(&context, "ABC", 2, 3); expect(1, 3);
    hud_tilemap_put_s_layer(&context, "ABC", 0, 2, 3); expect(0, 3);
    hud_tilemap_nput_s(&context, bytes, 2, 2, 3); expect(1, 2);
    hud_tilemap_nput_s_layer(&context, bytes, 3, 0, 2, 3); expect(0, 3);
    assert(calls == 4);
    memset(full, 'A', sizeof(full));
    hud_tilemap_nput_s(&context, full, sizeof(full), 0, 0);
    assert(size == 40 && layer == 1 && !x && !y);
    for (unsigned i = 0; i < sizeof(full); ++i) assert(tiles[i] == 160);
    hud_tilemap_put_s(&context, "", 2, 3);
    assert(size == 0 && calls == 6); /* Original void/empty-string behavior. */
    puts("HUD text: mapping, layers, counted buffers and 40-character edge passed");
    return 0;
}

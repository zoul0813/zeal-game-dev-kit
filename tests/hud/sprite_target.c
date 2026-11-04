#include <zgdk/hud/sprite.h>
#include <zgdk/hud/tilemap.h>
/* The runner supplies data and executes this stack-local descriptor on Z80. */
gfx_sprite test_sprites[20];
const char *test_text;
uint8_t test_capacity, test_count;
uint8_t test_tiles[20];
gfx_error gfx_tilemap_load(gfx_context *ctx, void *data, uint8_t count,
                          uint8_t layer, uint8_t x, uint8_t y) {
    const uint8_t *from = data;
    uint8_t *to = test_tiles;
    (void)ctx; (void)layer; (void)x; (void)y;
    while (count--) *to++ = *from++;
    return GFX_SUCCESS;
}
void main(void) {
    hud_sprite_text_t row = { test_sprites, test_capacity, 12 };
    hud_sprite_put_s_centered(&row, test_text, 158, 120);
    hud_tilemap_nput_s_layer(0, test_text, test_count, 1, 0, 0);
}

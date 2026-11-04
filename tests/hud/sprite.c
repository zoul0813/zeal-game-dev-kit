#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zgdk/hud/sprite.h>
#include <zgdk/hud/tilemap.h>
#include <zgdk/ascii.h>

static uint8_t last_tiles[40];
gfx_error gfx_tilemap_load(gfx_context *ctx, void *tiles, uint8_t len, uint8_t layer, uint8_t x, uint8_t y) {
    (void)ctx; (void)layer; (void)x; (void)y;
    assert(len <= sizeof(last_tiles));
    memcpy(last_tiles, tiles, len);
    return GFX_SUCCESS;
}

/* Visibility follows hardware origin/height rules, rather than treating Y=0
 * as a hidden marker (which is incorrect for 32-pixel sprites). */
static void expect_hidden(const gfx_sprite *sprite) {
    int top = (int)sprite->y - 16;
    int height = (sprite->options & 2) ? 32 : 16;
    assert(top >= 480 || top + height <= 0);
}

int main(void) {
    gfx_sprite storage[6];
    gfx_sprite before, after;
    hud_sprite_text_t text = {storage + 1, 4, 12};
    unsigned i;
    memset(storage, 0x5a, sizeof(storage));
    before = storage[0]; after = storage[5];
    hud_sprite_put_s(&text, "A B", 10, 20);
    assert(storage[1].tile == 'A' && storage[1].x == 26 && storage[1].y == 36);
    expect_hidden(&storage[2]);
    assert(storage[3].tile == 'B' && storage[3].x == 50 && storage[3].y == 36);
    expect_hidden(&storage[4]);
    for (i = 1; i <= 4; ++i) {
        assert(storage[i].flags == 0x5a && storage[i].options == 0x5a5a);
    }
    /* No NUL and no spare readable byte: capacity must be tested before reading. */
    {
        const char full[4] = {'A','B','C','D'};
        hud_sprite_put_s_centered(&text, full, 158, 120);
        assert(storage[1].x == 150 && storage[1].y == 136);
        assert(storage[4].x == 186 && storage[4].tile == 'D');
        hud_sprite_nput_s(&text, full, 255, -8, 0);
        assert(storage[1].x == 8 && storage[4].x == 44);
    }
    hud_sprite_put_s_centered(&text, "ABCDEF", 158, 120);
    assert(storage[1].x == 150); /* center only the capacity-limited prefix */
    hud_sprite_put_s_centered(&text, "AB", 0, 0);
    assert(storage[1].x == 4 && storage[2].x == 16); /* partially offscreen */
    /* Both renderers must observe one font change, without per-view mappings. */
    ascii_map('A', 2, 200);
    hud_sprite_put_s(&text, "AB", 0, 0);
    hud_tilemap_nput_s_layer(NULL, "AB", 2, 1, 0, 0);
    assert(storage[1].tile == 200 && storage[2].tile == 201);
    assert(last_tiles[0] == storage[1].tile && last_tiles[1] == storage[2].tile);
    /* Named icon strings concatenate into one run in either renderer. */
#define LIFE_HEART_FULL "%"
#define LIFE_HEART_HALF "/"
    ascii_map(LIFE_HEART_FULL[0], 1, 17);
    ascii_map(LIFE_HEART_HALF[0], 1, 18);
    hud_sprite_put_s(&text, "A" LIFE_HEART_FULL LIFE_HEART_FULL LIFE_HEART_HALF, 0, 0);
    hud_tilemap_put_s(NULL, "A" LIFE_HEART_FULL LIFE_HEART_FULL LIFE_HEART_HALF, 0, 0);
    assert(storage[1].tile == 200 && storage[2].tile == 17);
    assert(storage[3].tile == 17 && storage[4].tile == 18);
    for (i = 0; i < 4; ++i) assert(last_tiles[i] == storage[i + 1].tile);
    text.spacing = 8;
    for (i = 0; i < 95; ++i) ASCII_MAP[i] = (uint8_t)(i + 100);
    hud_sprite_put_s(&text, "!A~", 0, 0);
    assert(storage[1].tile == 101 && storage[2].tile == 133 && storage[3].tile == 194);
    assert(storage[3].x == 32);
    {
        const char invalid[4] = {'A', 0, 31, (char)255};
        hud_sprite_nput_s(&text, invalid, 4, 0, 0);
        assert(storage[1].y == 16);
        for (i = 2; i <= 4; ++i) expect_hidden(&storage[i]);
    }
    hud_sprite_put_s(&text, "AAAA", 0, 0);
    hud_sprite_put_s(&text, "A", 0, 0);
    for (i = 2; i <= 4; ++i) expect_hidden(&storage[i]);
    hud_sprite_hide(text.sprites, 4);
    for (i = 1; i <= 4; ++i) expect_hidden(&storage[i]);
    hud_sprite_put_s(&text, NULL, 0, 0);
    for (i = 1; i <= 4; ++i) expect_hidden(&storage[i]);
    hud_sprite_nput_s(&text, "A", 0, 0, 0);
    for (i = 1; i <= 4; ++i) expect_hidden(&storage[i]);
    /* Exercise both heights, and prove hiding preserves visible-position X,
     * tile, flags and options while allowing the next write to show the glyph. */
    for (unsigned tall = 0; tall <= 1; ++tall) {
        storage[1].options = tall ? 2 : 0;
        storage[1].flags = 0xa8;
        hud_sprite_put_s(&text, "A", 20, 40);
        gfx_sprite saved = storage[1];
        hud_sprite_hide(text.sprites, 1);
        expect_hidden(&storage[1]);
        assert(storage[1].x == saved.x && storage[1].tile == saved.tile);
        assert(storage[1].flags == saved.flags && storage[1].options == saved.options);
        hud_sprite_put_s(&text, "A", 20, 40);
        assert(memcmp(&storage[1], &saved, sizeof(saved)) == 0);
        hud_sprite_put_s(&text, " ", 20, 40);
        expect_hidden(&storage[1]);
        assert(storage[1].flags == saved.flags && storage[1].options == saved.options);
    }
    hud_sprite_put_s(NULL, "A", 0, 0);
    hud_sprite_put_s_centered(NULL, "A", 0, 0);
    hud_sprite_nput_s(NULL, "A", 1, 0, 0);
    hud_sprite_hide(NULL, 255);
    text.capacity = 0;
    hud_sprite_put_s(&text, "A", 0, 0);
    text.sprites = NULL; text.capacity = 4;
    hud_sprite_put_s(&text, "A", 0, 0);
    assert(memcmp(&storage[0], &before, sizeof(before)) == 0);
    assert(memcmp(&storage[5], &after, sizeof(after)) == 0);
    puts("sprite HUD: bounded text, centering, mapping, hiding and metadata preservation passed");
    return 0;
}

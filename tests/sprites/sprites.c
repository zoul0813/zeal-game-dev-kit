#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/sprites.h"

static gfx_sprite hardware[GFX_SPRITES_COUNT];
static uint8_t last_count;
static gfx_error render_error;

gfx_error gfx_sprite_render_array(gfx_context *ctx, uint8_t first,
                                 const gfx_sprite *sprites, uint8_t count) {
    if (!ctx || !sprites || first + count > GFX_SPRITES_COUNT) return GFX_INVALID_ARG;
    last_count = count;
    if (render_error != GFX_SUCCESS) return render_error;
    memcpy(hardware + first, sprites, count * sizeof(gfx_sprite));
    return GFX_SUCCESS;
}

static void expect_hidden(unsigned first, unsigned count) {
    for (unsigned i = first; i < first + count; ++i) {
        assert(hardware[i].x == 0 && hardware[i].y == 0);
        assert(hardware[i].tile == 0 && hardware[i].flags == 0);
        assert(hardware[i].options == 0);
    }
}

int main(void) {
    gfx_context ctx = {0};
    gfx_sprite arena[GFX_SPRITES_COUNT];
    gfx_sprite sprite = {.x=40, .y=40, .tile=7, .flags=1, .options=2};

    Point point;
    sprite_get_point(&sprite, &point);
    assert(point.x == 40 && point.y == 40);
    point.x = 300;
    point.y = 511;
    sprite_set_point(&sprite, &point);
    assert(sprite.x == 300 && sprite.y == 511);
    sprite_set_xy(&sprite, 40, 40);
    assert(sprite.x == 40 && sprite.y == 40);
    assert(sprite.tile == 7 && sprite.flags == 1 && sprite.options == 2);

    assert(sprites_register(sprite) == NULL);
    sprites_deregister();
    assert(sprites_render(&ctx) == GFX_INVALID_ARG);
    assert(sprites_register_arena(arena, GFX_SPRITES_COUNT) == GFX_SUCCESS);
    for (unsigned i = 0; i < GFX_SPRITES_COUNT; ++i)
        assert(sprites_register(sprite) == arena + i);
    assert(sprites_register(sprite) == NULL);
    assert(sprites_register_arena(NULL, 4) == GFX_INVALID_ARG);
    assert(sprites_register_arena(arena, 0) == GFX_INVALID_ARG);
    assert(sprites_register_arena(arena, 129) == GFX_INVALID_ARG);
    assert(sprites_register_arena(arena, 255) == GFX_INVALID_ARG);
    assert(sprites_register(sprite) == NULL); /* Invalid registration preserves state. */
    assert(sprites_render(&ctx) == GFX_SUCCESS && last_count == 128);
    assert(hardware[127].tile == 7);

    sprites_deregister();
    sprites_deregister();
    assert(sprites_register(sprite) == arena);
    assert(sprites_register(sprite) == arena + 1);
    render_error = GFX_INVALID_ARG;
    assert(sprites_render(&ctx) == render_error && last_count == 128);
    assert(hardware[127].tile == 7); /* Failure leaves cleanup pending. */
    render_error = GFX_SUCCESS;
    assert(sprites_render(&ctx) == GFX_SUCCESS && last_count == 128);
    assert(hardware[0].tile == 7 && hardware[1].tile == 7);
    expect_hidden(2, 126);
    assert(sprites_render(&ctx) == GFX_SUCCESS && last_count == 2);

    sprites_deregister();
    assert(sprites_render(NULL) == GFX_INVALID_ARG);
    assert(sprites_render(&ctx) == GFX_SUCCESS && last_count == 2);
    expect_hidden(0, 128);
    assert(sprites_render(&ctx) == GFX_SUCCESS && last_count == 0);

    /* Repeated reloads reclaim every slot, including in a one-slot arena. */
    assert(sprites_register_arena(arena, 1) == GFX_SUCCESS);
    for (unsigned i = 0; i < 300; ++i) {
        assert(sprites_register(sprite) == arena);
        assert(sprites_register(sprite) == NULL);
        assert(sprites_render(&ctx) == GFX_SUCCESS && last_count == 1);
        sprites_deregister();
        assert(sprites_render(&ctx) == GFX_SUCCESS && last_count == 1);
        expect_hidden(0, 1);
    }
    puts("sprite regression checks passed");
    return 0;
}

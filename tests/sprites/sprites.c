#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/sprites.h"

static gfx_sprite hardware[GFX_SPRITES_COUNT];
static uint8_t last_count;
static uint8_t dma_available;
extern gfx_sprite *sprites_arena;
extern uint8_t sprites_registered, sprites_rendered;
void sprites_dma_prepare(void) {}
uint8_t sprites_dma_upload(void) {
    if (!dma_available) return 0;
    unsigned count = sprites_registered > sprites_rendered ? sprites_registered : sprites_rendered;
    memcpy(hardware, sprites_arena, count * sizeof(gfx_sprite));
    last_count = count;
    return 1;
}
static gfx_error render_error;
static unsigned clear_calls;
static uint8_t fail_slot = GFX_SPRITES_COUNT;

gfx_error gfx_sprite_render(gfx_context *ctx, uint8_t slot, const gfx_sprite *sprite) {
    ++clear_calls;
    if (!ctx || !sprite || slot >= GFX_SPRITES_COUNT) return GFX_INVALID_ARG;
    if (render_error != GFX_SUCCESS) return render_error;
    if (slot == fail_slot) return GFX_INVALID_ARG;
    hardware[slot] = *sprite;
    return GFX_SUCCESS;
}

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
    memset(hardware, 0xa5, sizeof(hardware));
    assert(sprites_clear(&ctx) == GFX_SUCCESS); /* No arena required. */
    assert(clear_calls == GFX_SPRITES_COUNT);
    expect_hidden(0, GFX_SPRITES_COUNT);
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

    {
        gfx_sprite saved[GFX_SPRITES_COUNT];
        memcpy(saved, arena, sizeof(saved));
        clear_calls = 0;
        assert(sprites_clear(NULL) == GFX_INVALID_ARG && clear_calls == 1);
        assert(hardware[0].tile == 7 && hardware[127].tile == 7);
        render_error = GFX_INVALID_ARG;
        clear_calls = 0;
        assert(sprites_clear(&ctx) == render_error && clear_calls == 1);
        render_error = GFX_SUCCESS;
        fail_slot = 3;
        clear_calls = 0;
        assert(sprites_clear(&ctx) == GFX_INVALID_ARG && clear_calls == 4);
        expect_hidden(0, 3);
        assert(hardware[3].tile == 7 && hardware[127].tile == 7);
        fail_slot = GFX_SPRITES_COUNT;
        clear_calls = 0;
        assert(sprites_clear(&ctx) == GFX_SUCCESS && clear_calls == GFX_SPRITES_COUNT);
        expect_hidden(0, GFX_SPRITES_COUNT);
        assert(!memcmp(saved, arena, sizeof(saved)));
        assert(sprites_register(sprite) == NULL); /* Registrations survive clear. */
        assert(sprites_render(&ctx) == GFX_SUCCESS && last_count == GFX_SPRITES_COUNT);
        assert(!memcmp(hardware, saved, sizeof(saved)));
    }

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
    /* DMA preserves ordering, full capacity, and cleanup after shrinking. */
    dma_available = 1;
    assert(sprites_register_arena(arena, GFX_SPRITES_COUNT) == GFX_SUCCESS);
    for (unsigned i = 0; i < GFX_SPRITES_COUNT; ++i) {
        sprite.x = 40 + i;
        sprite.tile = i;
        assert(sprites_register(sprite) == arena + i);
    }
    render_error = GFX_INVALID_ARG; /* CPU backend must not be used. */
    assert(sprites_render(&ctx) == GFX_SUCCESS);
    assert(!memcmp(hardware, arena, sizeof(arena)));
    sprites_deregister();
    assert(sprites_register(sprite) == arena);
    assert(sprites_render(&ctx) == GFX_SUCCESS);
    expect_hidden(1, 127);
    assert(sprites_render(NULL) == GFX_INVALID_ARG);
    sprites_deregister();
    assert(sprites_render(&ctx) == GFX_SUCCESS);
    expect_hidden(0, 128);
    render_error = GFX_SUCCESS;
    assert(sprites_render(&ctx) == GFX_SUCCESS);
    puts("sprite regression checks passed");
    return 0;
}

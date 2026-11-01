#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/tilemap.h"

static gfx_context context;
static uint8_t vram[2][40][80], source[80 * 40], old_tiles[1];
static Tilemap prior, target, saved;
static Tilemap *previous;
static unsigned calls;
static int fail_at;
static uint8_t w, h, layer, dx, dy;

static void unchanged(void) {
    assert(_TILEMAP == previous);
    assert(target.tiles == saved.tiles);
    assert(target.rect.x == saved.rect.x && target.rect.y == saved.rect.y);
    assert(target.rect.w == saved.rect.w && target.rect.h == saved.rect.h);
}

gfx_error gfx_tilemap_load(gfx_context *ctx, void *tiles, uint8_t size,
                            uint8_t l, uint8_t x, uint8_t y) {
    unsigned row = calls++;
    unchanged();  /* No registration or descriptor mutation before every upload succeeds. */
    assert(ctx == &context && l == layer && x == dx && y == dy + row);
    assert(size == w && tiles == source + row * w);
    assert(size && x + size <= 80 && y < 40 && l < 2);
    if ((int)row == fail_at) return GFX_INVALID_ARG;
    memcpy(&vram[l][y][x], tiles, size);
    return GFX_SUCCESS;
}

static void reset(uint8_t width, uint8_t height, uint8_t l, uint8_t x, uint8_t y,
                   int failure, int reload) {
    w = width; h = height; layer = l; dx = x; dy = y;
    calls = 0; fail_at = failure;
    memset(vram, 0xcc, sizeof(vram));
    target.tiles = old_tiles;
    target.rect.x = 3; target.rect.y = 4;
    target.rect.w = 5; target.rect.h = 6;
    saved = target;
    previous = reload ? &target : &prior;
    _TILEMAP = previous;
}

static void check_vram(unsigned uploaded) {
    unsigned l, x, y;
    for (l = 0; l < 2; l++) for (y = 0; y < 40; y++) for (x = 0; x < 80; x++) {
        uint8_t expected = 0xcc;
        if (l == layer && y >= dy && y < dy + uploaded && x >= dx && x < dx + w)
            expected = source[(y - dy) * w + x - dx];
        assert(vram[l][y][x] == expected);
    }
}

int main(void) {
    unsigned width, height, l, i, successes = 0;
    Point point;
    for (i = 0; i < sizeof(source); i++) source[i] = (uint8_t)(i * 37 + 19);
    /* Every supported dimension, both layers, offset to the bottom/right edge. */
    for (l = 0; l < 2; l++) for (width = 1; width <= 80; width++)
        for (height = 1; height <= 40; height++) {
            reset(width, height, l, 80 - width, 40 - height, -1, successes & 1);
            assert(tilemap_load(&context, &target, source, w, h, layer, dx, dy) == ERR_SUCCESS);
            assert(calls == h && _TILEMAP == &target && target.tiles == source);
            assert(target.rect.x == dx && target.rect.y == dy);
            assert(target.rect.w == w && target.rect.h == h);
            assert(tilemap_get_xy(w - 1, h - 1) == source[w * h - 1]);
            point.x = dx + w - 1; point.y = dy + h - 1;
            assert(tilemap_get(&point) == source[w * h - 1]);
            check_vram(h);
            successes++;
        }
    /* Origin-zero room loading remains supported, too. */
    reset(20, 15, 0, 0, 0, -1, 0);
    assert(tilemap_load(&context, &target, source, w, h, layer, dx, dy) == ERR_SUCCESS);
    check_vram(h);
    /* Fail every row, including reloading the currently registered descriptor. */
    for (l = 0; l < 2; l++) for (i = 0; i < 15; i++) {
        reset(20, 15, 1, 7, 9, i, l);
        assert(tilemap_load(&context, &target, source, w, h, layer, dx, dy) == ERR_FAILURE);
        assert(calls == i + 1);
        unchanged(); check_vram(i);
    }
    {
        static const uint8_t invalid[][5] = {
            {0,15,0,0,0}, {20,0,0,0,0}, {20,15,2,0,0},
            {81,1,0,0,0}, {1,41,0,0,0}, {20,15,0,61,0},
            {20,15,0,0,26}, {1,1,0,80,0}, {1,1,0,0,40},
            {255,255,0,0,0}, {1,1,0,255,0}, {1,1,0,0,255},
            {2,2,0,255,255}
        };
        for (i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
            const uint8_t *v = invalid[i];
            reset(20,15,0,0,0,-1,0);
            assert(tilemap_load(&context,&target,source,v[0],v[1],v[2],v[3],v[4]) == ERR_INVALID_PARAMETER);
            assert(!calls); unchanged(); check_vram(0);
        }
    }
    reset(20,15,0,0,0,-1,0);
    assert(tilemap_load(NULL,&target,source,w,h,layer,dx,dy) == ERR_INVALID_PARAMETER);
    assert(tilemap_load(&context,NULL,source,w,h,layer,dx,dy) == ERR_INVALID_PARAMETER);
    assert(tilemap_load(&context,&target,NULL,w,h,layer,dx,dy) == ERR_INVALID_PARAMETER);
    assert(!calls); unchanged(); check_vram(0);
    puts("tilemap load: 6401 rectangles, 30 injected failures and 16 invalid calls passed");
    return 0;
}

#include <assert.h>
#include <stdio.h>
#include <zgdk/physics/tilemap.h>
#include <zgdk/tilemap.h>
TILEFLAGS_DEFINE_CACHE(3);
Tilemap *_TILEMAP;
int main(void) {
    uint8_t tiles[8] = {0,1,2,3, 1,2,3,0};
    uint8_t flags[256] = {0};
    Tilemap map = { .tiles = tiles, .rect = { .w = 4, .h = 2 } };
    /* Lookup tables carry arbitrary game classifications, not platformer enums. */
    flags[1] = 0x20; flags[2] = 0x40; flags[3] = 0x80;
    _TILEMAP = &map;
    tileflags_register(flags);
    tileflags_set_outside(0x10);
    tileflags_refresh();
    assert(tileflags_at_tile(1,0) == 0x20 && tileflags_fast(32,16) == 0x80);
    assert(tileflags_at(-1,0) == 0x10 && tileflags_at_tile(4,0) == 0x10);
    flags[1] = 0x60;  /* table edits need no cache rebuild */
    assert(tileflags_at(16,0) == 0x60);
    tileflags_set_outside(TF_NONE);
    assert(tileflags_at(1000,1000) == TF_NONE);
    body_t shot;
    body_init(&shot, 0, 0, 4, 4);
    shot.vx = PHYSICS_FP(1);
    body_set_touch_mask(0);
    body_step(&shot);
    assert(shot.x == PHYSICS_FP(1));  /* no platformer or automatic gravity */
    _TILEMAP = NULL;
    shot.vy = PHYSICS_FP(1);
    body_step(&shot);
    assert(shot.x == PHYSICS_FP(2) && shot.y == PHYSICS_FP(1));
    puts("custom tile flags and gravity-free collision: passed");
    return 0;
}

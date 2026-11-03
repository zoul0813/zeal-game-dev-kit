#include <assert.h>
#include <stdio.h>
#include <zgdk/tilemap/flags.h>
#include <zgdk/tilemap.h>
TILEFLAGS_DEFINE_CACHE(3);
Tilemap *_TILEMAP;
int main(void) {
    uint8_t tiles[8] = {0,1,2,3, 1,2,3,0};
    uint8_t flags[256] = {0};
    Tilemap map = {0};
    map.tiles = tiles;
    map.rect.w = 4;
    map.rect.h = 2;
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
    puts("tile flags: passed without physics");
    return 0;
}

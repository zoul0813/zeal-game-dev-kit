#include <assert.h>
#include <stdio.h>
#include "zgdk/types.h"
int main(void)
{
    assert(TILE_BYTE_OFFSET(TILESET_4BIT, 0) == 0);
    assert(TILE_BYTE_OFFSET(TILESET_4BIT, 1) == 128);
    assert(TILE_BYTE_OFFSET(TILESET_4BIT, 255) == 32640);
    assert(TILE_BYTE_OFFSET(TILESET_8BIT, 0) == 0);
    assert(TILE_BYTE_OFFSET(TILESET_8BIT, 1) == 256);
    assert(TILE_BYTE_OFFSET(TILESET_8BIT, 255) == 65280);
    assert(TILE_BYTE_OFFSET(2 + 2, 1 + 2) == 384);
    assert(TILE_BYTE_OFFSET(4 + 4, 1 + 2) == 768);
    assert(1 + TILE_BYTE_OFFSET(TILESET_4BIT, 1) == 129);
    unsigned index = 1, bits = TILESET_4BIT;
    assert(TILE_BYTE_OFFSET(bits++, index++) == 128);
    assert(index == 2 && bits == TILESET_4BIT + 1);
    puts("tile offset checks passed");
}

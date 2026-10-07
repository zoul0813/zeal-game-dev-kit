#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/tiles.h"

static uint16_t scroll_x, scroll_y;
uint16_t tilemap_scroll_get_x(uint8_t layer) { (void)layer; return scroll_x; }
uint16_t tilemap_scroll_get_y(uint8_t layer) { (void)layer; return scroll_y; }

static void expect_tile(Tile *tile, int x, int y) {
    assert(tile->x == x && tile->y == y);
    assert(tile->rect.x == (x + 1) * 16);
    assert(tile->rect.y == (y + 1) * 16);
    assert(tile->rect.w == 16 && tile->rect.h == 16);
}

int main(void) {
    Tile tile;
    TileCollision c;
    Rect r;
    Direction d;

    /* Bricked's 8x8 ball: center (17,17), not the old (9,9). */
    rect_set(r, 21, 21, 8, 8);
    assert(tile_get(&r, &tile));
    expect_tile(&tile, 1, 1);
    d.x = 1; d.y = 1;
    tile_collide_ex(&r, &d, &c);
    assert(c.hit_h && c.hit_v && c.hit_c);
    assert(c.edge_h == EdgeLeft && c.edge_v == EdgeTop);
    assert(c.edge_c == (EdgeLeft | EdgeTop));
    expect_tile(&c.tile_h, 1, 1);
    expect_tile(&c.tile_v, 1, 1);
    expect_tile(&c.tile_c, 1, 1);
    d.x = -1; d.y = -1;
    tile_collide_ex(&r, &d, &c);
    assert(c.edge_h == EdgeRight && c.edge_v == EdgeBottom);
    expect_tile(&c.tile_h, 0, 1);
    expect_tile(&c.tile_v, 1, 0);
    expect_tile(&c.tile_c, 0, 0);

    /* A 32x24 rectangle: center (32,36), leading edges (16,24)/(48,48). */
    rect_set(r, 48, 48, 32, 24);
    assert(tile_get(&r, &tile));
    expect_tile(&tile, 2, 2);
    d.x = -1; d.y = -1;
    tile_collide_ex(&r, &d, &c);
    expect_tile(&c.tile_h, 1, 2);
    expect_tile(&c.tile_v, 2, 1);
    expect_tile(&c.tile_c, 1, 1);
    d.x = 1; d.y = 1;
    tile_collide_ex(&r, &d, &c);
    expect_tile(&c.tile_h, 3, 2);
    expect_tile(&c.tile_v, 2, 3);
    expect_tile(&c.tile_c, 3, 3);

    /* Odd dimensions choose the integer center toward bottom/right. */
    rect_set(r, 18, 19, 5, 7);
    assert(tile_get(&r, &tile));
    expect_tile(&tile, 1, 1);

    /* All nine movement combinations preserve existing 16x16 probes. */
    rect_set(r, 32, 32, 16, 16);
    assert(tile_get(&r, &tile));
    expect_tile(&tile, 1, 1);
    assert(tile_get_edge(&r, EdgeLeft, &tile));
    expect_tile(&tile, 0, 1);
    assert(tile_get_edge(&r, EdgeRight, &tile)); expect_tile(&tile, 2, 1);
    assert(tile_get_edge(&r, EdgeTop, &tile)); expect_tile(&tile, 1, 0);
    assert(tile_get_edge(&r, EdgeBottom, &tile)); expect_tile(&tile, 1, 2);
    assert(tile_get_edge(&r, EdgeTop | EdgeLeft, &tile)); expect_tile(&tile, 0, 0);
    scroll_x = 7; scroll_y = 9;
    assert(tile_get_edge(&r, EdgeRight, &tile));
    assert(tile.x == 2 && tile.y == 1 && tile.rect.x == 55 && tile.rect.y == 41);
    scroll_x = scroll_y = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            d.x = dx; d.y = dy;
            memset(&c, 0xff, sizeof(c));
            tile_collide_ex(&r, &d, &c);
            assert(c.hit_h == (dx != 0));
            assert(c.hit_v == (dy != 0));
            assert(c.hit_c == (dx != 0 || dy != 0));
            assert(c.edge_h == (dx < 0 ? EdgeRight : dx > 0 ? EdgeLeft : EdgeNone));
            assert(c.edge_v == (dy < 0 ? EdgeBottom : dy > 0 ? EdgeTop : EdgeNone));
            if (dx) expect_tile(&c.tile_h, dx > 0 ? 2 : 1, 1);
            if (dy) expect_tile(&c.tile_v, 1, dy > 0 ? 2 : 1);
            if (dx || dy) {
                expect_tile(&c.tile_c, dx > 0 ? 2 : 1, dy > 0 ? 2 : 1);
                assert(tile_collide(&r, &d, &tile) == c.edge_c);
                expect_tile(&tile, dx > 0 ? 2 : 1, dy > 0 ? 2 : 1);
            } else assert(tile_collide(&r, &d, &tile) == EdgeNone);
        }
    }

    /* Small ball at the origin and outside the tilemap. */
    rect_set(r, 8, 8, 8, 8);
    d.x = -1; d.y = -1;
    tile_collide_ex(&r, &d, &c);
    assert(c.hit_h && c.hit_v && c.hit_c);
    expect_tile(&c.tile_c, 0, 0);
    rect_set(r, 4, 4, 8, 8);
    tile_collide_ex(&r, &d, &c);
    assert(!c.hit_h && !c.hit_v && !c.hit_c);
    rect_set(r, SCROLL_WIDTH, SCROLL_HEIGHT, 8, 8);
    d.x = 1; d.y = 1;
    tile_collide_ex(&r, &d, &c);
    assert(!c.hit_h && !c.hit_v && !c.hit_c);
    puts("tile regression checks passed");
    return 0;
}

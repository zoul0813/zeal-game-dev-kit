#include "support.h"
#include <zgdk/tilemap.h>
#include <zgdk/tilemap/scroll.h>
#include <assert.h>
#include <string.h>

uint16_t test_scroll_x, test_scroll_y;
uint8_t test_scroll_layer;
unsigned test_scroll_count, test_wait_count, test_upload_count, test_place_count;
uint8_t test_uploaded[512];
uint16_t test_uploaded_size, test_uploaded_from;
uint8_t test_placed_tile[2], test_placed_x[2], test_placed_y[2], test_placed_layer[2];
uint8_t test_graphics_error;

void test_reset_graphics(void) {
    test_scroll_count = test_wait_count = test_upload_count = test_place_count = 0;
    test_graphics_error = 0;
}
void tilemap_scroll(uint8_t layer,uint16_t x,uint16_t y) {
    assert(layer <= 1 && x < 1280 && y < 640);
    test_scroll_layer=layer; test_scroll_x=x; test_scroll_y=y; test_scroll_count++;
}
gfx_error gfx_wait_vblank(gfx_context* ctx) { assert(ctx);test_wait_count++;return test_graphics_error; }
gfx_error gfx_wait_end_vblank(gfx_context* ctx) { assert(ctx);return test_graphics_error; }
gfx_error gfx_tileset_load_none(gfx_context* ctx,uint8_t* data,uint16_t size,uint16_t from,uint8_t pal_offset,uint8_t opacity) {
    assert(ctx && size<=sizeof(test_uploaded));
    assert(pal_offset == 0 && opacity == 0);
    memcpy(test_uploaded,data,size);test_uploaded_size=size;test_uploaded_from=from;test_upload_count++;
    return test_graphics_error;
}
gfx_error gfx_tilemap_place(gfx_context* ctx,uint8_t tile,uint8_t layer,uint8_t x,uint8_t y) {
    assert(ctx && x<80 && y<40 && layer<2);
    unsigned n=test_place_count++ % 2;
    test_placed_tile[n]=tile;test_placed_x[n]=x;test_placed_y[n]=y;test_placed_layer[n]=layer;
    return test_graphics_error;
}
gfx_error gfx_tilemap_load(gfx_context* ctx,void* data,uint8_t size,uint8_t layer,uint8_t x,uint8_t y) {
    (void)data;assert(ctx && x+size<=80 && y<40 && layer<2);return test_graphics_error;
}

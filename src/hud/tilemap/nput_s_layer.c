#include "zgdk/hud/tilemap.h"
#include "zgdk/ascii.h"

void hud_tilemap_nput_s_layer(gfx_context *ctx, const char *str, uint8_t len, uint8_t layer, uint8_t x, uint8_t y) {
    uint8_t line[40];
    for (uint8_t i = 0; i < len; ++i) {
        line[i] = ascii_to(str[i]);
    }
    gfx_tilemap_load(ctx, line, len, layer, x, y);
}

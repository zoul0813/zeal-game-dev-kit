#include "zgdk/hud/tilemap.h"
#include "zgdk/types.h"

void hud_tilemap_nput_s(gfx_context *ctx, const char *str, uint8_t len, uint8_t x, uint8_t y) {
    hud_tilemap_nput_s_layer(ctx, str, len, LAYER1, x, y);
}

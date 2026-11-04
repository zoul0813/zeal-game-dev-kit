#include "zgdk/hud/tilemap.h"
#include "zgdk/utils/_strlen.h"

void hud_tilemap_put_s_layer(gfx_context *ctx, const char *str, uint8_t layer, uint8_t x, uint8_t y) {
    hud_tilemap_nput_s_layer(ctx, str, strlen(str), layer, x, y);
}

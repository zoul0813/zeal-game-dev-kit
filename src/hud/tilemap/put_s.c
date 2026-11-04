#include "zgdk/hud/tilemap.h"
#include "zgdk/utils/_strlen.h"
#include "zgdk/types.h"

void hud_tilemap_put_s(gfx_context *ctx, const char *str, uint8_t x, uint8_t y) {
    hud_tilemap_nput_s_layer(ctx, str, strlen(str), LAYER1, x, y);
}

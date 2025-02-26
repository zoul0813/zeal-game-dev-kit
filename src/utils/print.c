#include <zvb_gfx.h>
#include "zgdk/utils/_strlen.h"
#include "zgdk/types.h"
#include "zgdk/utils.h"
#include "zgdk/ascii.h"

inline void print_string(gfx_context* ctx, const char* str, uint8_t x, uint8_t y)
{
    return nprint_string_layer(ctx, str, strlen(str), LAYER1, x, y);
}

inline void print_string_layer(gfx_context* ctx, const char* str, uint8_t layer, uint8_t x, uint8_t y)
{
    return nprint_string_layer(ctx, str, strlen(str), layer, x, y);
}

inline void nprint_string(gfx_context* ctx, const char* str, uint8_t len, uint8_t x, uint8_t y)
{
    return nprint_string_layer(ctx, str, len, LAYER1, x, y);
}

void nprint_string_layer(gfx_context* ctx, const char* str, uint8_t len, uint8_t layer, uint8_t x, uint8_t y)
{
    uint8_t line[40];
    for(uint8_t i = 0; i < len; i++) {
        line[i] = ascii_to(str[i]);
    }
    gfx_tilemap_load(ctx, line, len, layer, x, y);
}

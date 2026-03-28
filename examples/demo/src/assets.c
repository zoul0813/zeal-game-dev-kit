#include <stdio.h>
#include <zvb_gfx.h>
#include "assets.h"

gfx_error load_palette(gfx_context* ctx) {
    // Load the palette
    const size_t palette_size = &_tileset_ztp_end - &_tileset_ztp_start;
    return gfx_palette_load(ctx, &_tileset_ztp_start, palette_size, 0);
}

gfx_error load_tileset(gfx_context* ctx, gfx_tileset_options* options) {
    const size_t size = &_tileset_zts_end - &_tileset_zts_start;
    return gfx_tileset_load(ctx, &_tileset_zts_start, size, options);
}

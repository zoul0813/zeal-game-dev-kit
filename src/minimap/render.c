#include <zgdk/minimap/render.h>
#include <zgdk/minimap/state.h>
#include "internal.h"

#include <string.h>

#define MINIMAP_TILE_PIXELS 16
#define MINIMAP_TILE_BYTES 256
#define MINIMAP_BUFFER_SIZE 512

static uint8_t minimap_tiles[MINIMAP_BUFFER_SIZE];

static uint8_t valid_config(gfx_context* ctx, const minimap_render_config_t* config) {
    if (ctx == NULL || config == NULL || (ctx->bpp != 4 && ctx->bpp != 8)) return 0;
    if (config->layer > 1 || config->first_tile > 254 || config->position.x > 78 || config->position.y >= 40) return 0;
    if (ctx->bpp == 4 && (config->colors.unvisited > 15 || config->colors.visited > 15 ||
        config->colors.special > 15 || config->colors.current > 15)) return 0;
    return 1;
}

uint8_t minimap_render_valid(gfx_context* ctx, const minimap_render_config_t* config,
    const minimap_state_config_t* state) {
    return valid_config(ctx, config) && state != NULL
        && state->size.w && state->size.w <= 16
        && state->size.h && state->size.h <= 8;
}

static void draw_pixel(uint8_t x, uint8_t y, uint8_t color, uint8_t bpp) {
    uint16_t offset = (uint16_t)(x / MINIMAP_TILE_PIXELS) * MINIMAP_TILE_BYTES +
                      (uint16_t)y * MINIMAP_TILE_PIXELS + x % MINIMAP_TILE_PIXELS;
    if (bpp == 8) minimap_tiles[offset] = color;
    else if (offset & 1) minimap_tiles[offset >> 1] = (minimap_tiles[offset >> 1] & 0xF0) | color;
    else minimap_tiles[offset >> 1] = (minimap_tiles[offset >> 1] & 0x0F) | (color << 4);
}

static void draw_room(uint8_t x, uint8_t y, uint8_t color, uint8_t bpp) {
    uint8_t px = x * 2;
    uint8_t py = y * 2;
    draw_pixel(px, py, color, bpp);
    draw_pixel(px + 1, py, color, bpp);
    draw_pixel(px, py + 1, color, bpp);
    draw_pixel(px + 1, py + 1, color, bpp);
}

gfx_error minimap_place(gfx_context* ctx, const minimap_render_config_t* config) {
    if (!valid_config(ctx, config)) return GFX_INVALID_ARG;
    gfx_error err = gfx_tilemap_place(ctx, config->first_tile, config->layer, config->position.x, config->position.y);
    if (err != GFX_SUCCESS) return err;
    return gfx_tilemap_place(ctx, config->first_tile + 1, config->layer, config->position.x + 1, config->position.y);
}

gfx_error minimap_render(gfx_context* ctx, const minimap_render_config_t* config) {
    const minimap_state_config_t* state = minimap_get_config();
    if (!minimap_render_valid(ctx, config, state)) return GFX_INVALID_ARG;
    Vector2_u8 position;
    position = minimap_get_position();
    uint16_t tile_bytes = ctx->bpp == 8 ? 256 : 128;
    uint8_t background = config->colors.unvisited;
    if (ctx->bpp == 4) background |= background << 4;
    memset(minimap_tiles, background, tile_bytes * 2);
    for (uint8_t y = 0; y < state->size.h; y++) {
        for (uint8_t x = 0; x < state->size.w; x++) {
            if (minimap_room_visited(x, y)) {
                uint8_t color = minimap_room_is_special(x, y) ? config->colors.special : config->colors.visited;
                draw_room(x, y, color, ctx->bpp);
            }
        }
    }
    draw_room(position.x, position.y, config->colors.current, ctx->bpp);
    return gfx_tileset_load_none(ctx, minimap_tiles, tile_bytes * 2,
        (uint16_t)config->first_tile * tile_bytes, 0, 0);
}

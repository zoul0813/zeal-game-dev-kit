#ifndef ZGDK_MINIMAP_RENDER_H
#define ZGDK_MINIMAP_RENDER_H

#include <zvb_gfx.h>
#include <zgdk/types.h>

typedef struct {
    uint8_t unvisited;
    uint8_t visited;
    uint8_t special;
    uint8_t current;
} minimap_colors_t;

typedef struct {
    Vector2_u8 position; /* screen tile coordinates */
    uint8_t layer;
    uint8_t first_tile;  /* reserves two consecutive tiles */
    minimap_colors_t colors;
} minimap_render_config_t;

/* Two-tile renderer: up to 16x8 rooms, two pixels per room, 4-bit or 8-bit.
 * Larger grids can use the state API with a game-owned renderer. */
gfx_error minimap_render(gfx_context* ctx, const minimap_render_config_t* config);
gfx_error minimap_place(gfx_context* ctx, const minimap_render_config_t* config);

#endif

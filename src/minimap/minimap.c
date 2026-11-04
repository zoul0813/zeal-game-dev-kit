#include <zgdk/minimap.h>
#include "internal.h"

static minimap_render_config_t render_config;
static uint8_t initialized;

gfx_error minimap_init(gfx_context* ctx, const minimap_state_config_t* state,
    const minimap_render_config_t* render, Vector2_u8 position) {
    if (!minimap_render_valid(ctx, render, state)) return GFX_INVALID_ARG;
    if (minimap_state_init(state, position) != ERR_SUCCESS) return GFX_INVALID_ARG;
    initialized = 0;
    render_config = *render;
    gfx_error err = minimap_render(ctx, &render_config);
    if (err != GFX_SUCCESS) return err;
    err = minimap_place(ctx, &render_config);
    initialized = err == GFX_SUCCESS;
    return err;
}

gfx_error minimap_set_position(gfx_context* ctx, Vector2_u8 position, uint8_t previous_is_special) {
    if (ctx == NULL || !initialized) return GFX_INVALID_ARG;
    minimap_move_backup_t backup;
    if (minimap_state_begin_move(position, previous_is_special, &backup) != ERR_SUCCESS) return GFX_INVALID_ARG;
    gfx_error err = minimap_render(ctx, &render_config);
    if (err != GFX_SUCCESS) minimap_state_rollback_move(&backup);
    return err;
}

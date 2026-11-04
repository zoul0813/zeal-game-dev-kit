#ifndef ZGDK_MINIMAP_H
#define ZGDK_MINIMAP_H

#include <zgdk/minimap/state.h>
#include <zgdk/minimap/render.h>

/* Convenience API combining state and rendering; configs are copied.
 * Invalid init parameters preserve an existing minimap and its bitsets.
 * A position-update error preserves position and flags, allowing the same call
 * to be retried. Partial graphics writes are not rolled back. */
gfx_error minimap_init(gfx_context* ctx, const minimap_state_config_t* state,
    const minimap_render_config_t* render, Vector2_u8 position);
gfx_error minimap_set_position(gfx_context* ctx, Vector2_u8 position, uint8_t previous_is_special);

#endif

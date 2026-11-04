#ifndef ZGDK_MINIMAP_INTERNAL_H
#define ZGDK_MINIMAP_INTERNAL_H
#include <zgdk/minimap/state.h>
#include <zgdk/minimap/render.h>

/* A move changes one special byte, one visited byte, and the current position.
 * Save only those fields, not the game-owned bitsets. */
typedef struct {
    Vector2_u8 position;
    uint16_t visited_index, special_index;
    uint8_t visited_byte, special_byte;
} minimap_move_backup_t;

zos_err_t minimap_state_begin_move(Vector2_u8 position, uint8_t previous_is_special,
    minimap_move_backup_t* backup);
void minimap_state_rollback_move(const minimap_move_backup_t* backup);
uint8_t minimap_render_valid(gfx_context* ctx, const minimap_render_config_t* config,
    const minimap_state_config_t* state);
#endif

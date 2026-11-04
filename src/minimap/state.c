#include <zgdk/minimap/state.h>
#include "internal.h"

#include <string.h>

static minimap_state_config_t minimap_config;
static Vector2_u8 minimap_position;
static uint8_t ready;

static uint16_t room_index(uint8_t x, uint8_t y) {
    return (uint16_t)y * minimap_config.size.w + x;
}

static uint8_t room_valid(uint8_t x, uint8_t y) {
    return ready && x < minimap_config.size.w && y < minimap_config.size.h;
}

uint8_t minimap_room_visited(uint8_t x, uint8_t y) {
    if (!room_valid(x, y)) return 0;
    uint16_t room = room_index(x, y);
    return minimap_config.visited[room >> 3] & (1 << (room & 7));
}

uint8_t minimap_room_is_special(uint8_t x, uint8_t y) {
    if (!room_valid(x, y)) return 0;
    uint16_t room = room_index(x, y);
    return minimap_config.special[room >> 3] & (1 << (room & 7));
}

static void set_visited(uint8_t x, uint8_t y) {
    uint16_t room = room_index(x, y);
    minimap_config.visited[room >> 3] |= (1 << (room & 7));
}

Vector2_u8 minimap_get_position(void) {
    return minimap_position;
}

const minimap_state_config_t* minimap_get_config(void) {
    return ready ? &minimap_config : NULL;
}

zos_err_t minimap_state_init(const minimap_state_config_t* config, Vector2_u8 position) {
    if (config == NULL || config->visited == NULL || config->special == NULL || config->visited == config->special ||
        !config->size.w || !config->size.h || position.x >= config->size.w || position.y >= config->size.h ||
        config->capacity < MINIMAP_STATE_BYTES(config->size.w, config->size.h)) return ERR_INVALID_PARAMETER;
    uint16_t bytes = MINIMAP_STATE_BYTES(config->size.w, config->size.h);
    uintptr_t visited = (uintptr_t)config->visited;
    uintptr_t special = (uintptr_t)config->special;
    /* Compare distances rather than adding byte counts to addresses, which
     * could wrap on Z80. Only the actively used spans must be disjoint. */
    if (visited < special ? special - visited < bytes : visited - special < bytes)
        return ERR_INVALID_PARAMETER;
    minimap_config = *config;
    memset(config->visited, 0, bytes);
    memset(config->special, 0, bytes);
    minimap_position = position;
    ready = 1;
    set_visited(position.x, position.y);
    return ERR_SUCCESS;
}

zos_err_t minimap_state_begin_move(Vector2_u8 position, uint8_t previous_is_special,
    minimap_move_backup_t* backup) {
    if (!room_valid(position.x, position.y)) return ERR_INVALID_PARAMETER;
    uint16_t room = room_index(minimap_position.x, minimap_position.y);
    if (backup != NULL) {
        backup->position = minimap_position;
        backup->special_index = room >> 3;
        backup->visited_index = room_index(position.x, position.y) >> 3;
        backup->special_byte = minimap_config.special[backup->special_index];
        backup->visited_byte = minimap_config.visited[backup->visited_index];
    }
    uint8_t bit = 1 << (room & 7);
    if (previous_is_special) minimap_config.special[room >> 3] |= bit;
    else minimap_config.special[room >> 3] &= ~bit;
    minimap_position = position;
    set_visited(position.x, position.y);
    return ERR_SUCCESS;
}

void minimap_state_rollback_move(const minimap_move_backup_t* backup) {
    minimap_position = backup->position;
    minimap_config.special[backup->special_index] = backup->special_byte;
    minimap_config.visited[backup->visited_index] = backup->visited_byte;
}

zos_err_t minimap_state_set_position(Vector2_u8 position, uint8_t previous_is_special) {
    return minimap_state_begin_move(position, previous_is_special, NULL);
}

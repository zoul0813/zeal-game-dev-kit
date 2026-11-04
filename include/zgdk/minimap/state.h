#ifndef ZGDK_MINIMAP_STATE_H
#define ZGDK_MINIMAP_STATE_H

#include <zgdk/types.h>
#include <zos_errors.h>

#define MINIMAP_STATE_BYTES(w, h) ((((uint16_t)(w) * (h)) + 7) / 8)

typedef struct {
    Size_u8 size;
    uint8_t* visited;
    uint8_t* special;
    uint16_t capacity; /* bytes available in each non-overlapping bitset */
} minimap_state_config_t;

/* Config is copied; the game owns both buffers for the minimap's lifetime. */
zos_err_t minimap_state_init(const minimap_state_config_t* config, Vector2_u8 position);
zos_err_t minimap_state_set_position(Vector2_u8 position, uint8_t previous_is_special);
const minimap_state_config_t* minimap_get_config(void);
Vector2_u8 minimap_get_position(void);
uint8_t minimap_room_visited(uint8_t x, uint8_t y);
uint8_t minimap_room_is_special(uint8_t x, uint8_t y);

#endif

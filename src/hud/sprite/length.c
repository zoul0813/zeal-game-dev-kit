#include "internal.h"

uint8_t hud_sprite_text_length(const hud_sprite_text_t *text, const char *str) {
    uint8_t len = 0, capacity;
    if (!text || !text->sprites || !str) return 0;
    /* Cache the bound: SDCC 4.4 size optimization otherwise advances the
     * descriptor field pointer on every loop iteration. */
    capacity = text->capacity;
    while (len < capacity && str[len]) ++len;
    return len;
}

#include "internal.h"

uint8_t hud_sprite_text_length(const hud_sprite_text_t *text, const char *str) {
    uint8_t len = 0;
    if (!text || !text->sprites || !str) return 0;
    while (len < text->capacity && str[len]) ++len;
    return len;
}

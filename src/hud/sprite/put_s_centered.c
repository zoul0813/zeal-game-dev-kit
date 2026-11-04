#include "internal.h"

void hud_sprite_put_s_centered(const hud_sprite_text_t *text, const char *str, int16_t x, int16_t y) {
    uint8_t len;
    if (!text) return;
    len = hud_sprite_text_length(text, str);
    x -= ((uint16_t)len * text->spacing) >> 1;
    hud_sprite_nput_s(text, str, len, x, y);
}

#include "internal.h"

void hud_sprite_put_s(const hud_sprite_text_t *text, const char *str, int16_t x, int16_t y) {
    hud_sprite_nput_s(text, str, hud_sprite_text_length(text, str), x, y);
}

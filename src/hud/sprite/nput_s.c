#include "internal.h"
#include "zgdk/ascii.h"

void hud_sprite_nput_s(const hud_sprite_text_t *text, const char *str, uint8_t len, int16_t x, int16_t y) {
    gfx_sprite *sprite;
    uint8_t count, spacing;
    uint16_t px = (uint16_t)x + 16, py = (uint16_t)y + 16;
    if (!text || !text->sprites) return;
    sprite = text->sprites;
    count = text->capacity;
    spacing = text->spacing;
    if (!str) len = 0;
    if (len > count) len = count;
    while (count--) {
        if (len) {
            uint8_t c = (uint8_t)*str++;
            --len;
            if (c > ' ' && c <= '~') {
                sprite->tile = ascii_to(c);
                sprite->x = px;
                sprite->y = py;
            } else {
                sprite->y = HUD_SPRITE_HIDDEN_Y;
            }
        } else {
            sprite->y = HUD_SPRITE_HIDDEN_Y;
        }
        ++sprite;
        px += spacing;
    }
}

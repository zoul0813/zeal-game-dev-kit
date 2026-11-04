#include "internal.h"

void hud_sprite_hide(gfx_sprite *sprites, uint8_t count) {
    if (!sprites) return;
    while (count--) {
        sprites->y = HUD_SPRITE_HIDDEN_Y;
        ++sprites;
    }
}

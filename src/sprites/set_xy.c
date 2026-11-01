#include "zgdk/sprites.h"

void sprite_set_xy(gfx_sprite *sprite, uint16_t x, uint16_t y) {
    sprite->x = x;
    sprite->y = y;
}

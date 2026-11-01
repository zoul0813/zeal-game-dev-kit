#include "zgdk/sprites.h"

void sprite_set_point(gfx_sprite *sprite, Point *point) {
    sprite->x = point->x;
    sprite->y = point->y;
}

#include "zgdk/sprites.h"

void sprite_get_point(gfx_sprite *sprite, Point *point) {
    point->x = sprite->x;
    point->y = sprite->y;
}

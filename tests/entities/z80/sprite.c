#include <zgdk/entities/sprite.h>
#include "arena.h"
int main(void) {
    sprite_entity_t e;
    gfx_sprite *s = reserve();
    if (sprite_entity_init(&e, s)) return 1;
    s->tile = 201;
    sprite_entity_sync(&e, 20, 30);
    if (s->x != 36 || s->y != 46 || s->tile != 201) return 2;
    e.tall = 1; e.flip = SPRITE_FLIP_X; e.ox = -2;
    entity_view_set(4, 5);
    sprite_entity_sync(&e, 20, 30);
    if (s->x != 30 || s->y != 41 || s->options != SPRITE_OPTION_32PX || s->flags != SPRITE_FLIP_X) return 3;
    e.hidden = 1;
    sprite_entity_sync(&e, 20, 30);
    if (s->y || s->options || !e.parked) return 4;
    s->tile = 42;
    sprite_entity_sync(&e, 20, 30);
    if (s->tile != 42) return 5;
    e.hidden = 0;
    entity_view_set(0, 0);
    if (entity_view_size(640, 480) != ERR_SUCCESS) return 6;
    sprite_entity_sync(&e, 500, 400);
    if (s->x != 514 || s->y != 416) return 7; /* ox stays -2. */
    if (entity_view_size(641, 480) != ERR_INVALID_PARAMETER) return 8;
    sprite_entity_sync(&e, 500, 400);
    if (s->y != 416) return 9;
    if (entity_view_size(320, 240) != ERR_SUCCESS) return 10;
    sprite_entity_sync(&e, 500, 400);
    if (s->y || s->options) return 11;
    return 0;
}

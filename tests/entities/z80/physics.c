#include <zgdk/entities/physics.h>
#include "arena.h"
int main(void) {
    physics_entity_t e;
    gfx_sprite *s = reserve();
    if (physics_entity_init(&e, s, 20, 30, 8, 9)) return 1;
    s->tile = 201;
    physics_entity_sync(&e);
    if (s->x != 36 || s->y != 46 || s->tile != 201) return 2;
    body_accelerate(&e.body, 16, 0);
    body_integrate(&e.body);
    physics_entity_sync(&e);
    if (s->x != 37 || e.body.vx != 16) return 3;
    e.visual.type = ENTITY_FREE;
    physics_entity_sync(&e);
    if (s->y || !e.visual.parked) return 4;
    return 0;
}

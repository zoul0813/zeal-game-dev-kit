#include <zgdk/entities/animated_physics.h>
#include "arena.h"
static const uint8_t frames[] = {201, 255};
static const anim_def def = {frames, 2, 1, 0};
int main(void) {
    animated_physics_entity_t e;
    gfx_sprite *s = reserve();
    if (animated_physics_entity_init(&e, s, 20, 30, 8, 9)) return 1;
    animated_physics_entity_play(&e, &def);
    s->tile = anim_tick(&e.anim);
    physics_entity_sync(&e.physics);
    if (s->x != 36 || s->y != 46 || s->tile != 201 || e.anim.index != 1) return 2;
    animated_physics_entity_play(&e, &def);
    if (s->tile != 201) return 3;
    s->tile = anim_tick(&e.anim);
    physics_entity_sync(&e.physics);
    if (!e.anim.done || s->tile != 255) return 4;
    animated_physics_entity_play(&e, NULL);
    physics_entity_sync(&e.physics);
    if (s->tile || e.anim.def) return 5;
    return 0;
}

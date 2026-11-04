#include <zgdk/entities/animated.h>
#include "arena.h"
static const uint8_t frames[] = {201, 42};
static const anim_def def = {frames, 2, 1, ANIM_LOOP};
int main(void) {
    animated_entity_t e;
    gfx_sprite *s = reserve();
    if (animated_entity_init(&e, s)) return 1;
    animated_entity_play(&e, &def);
    if (s->tile != 201) return 2;
    s->tile = anim_tick(&e.anim);
    if (e.anim.index != 1 || s->tile != 201) return 3;
    animated_entity_play(&e, &def);
    sprite_entity_sync(&e.visual, 20, 30);
    if (s->tile != 201 || s->x != 36 || s->y != 46) return 4;
    s->tile = anim_tick(&e.anim);
    sprite_entity_sync(&e.visual, 20, 30);
    if (s->tile != 42) return 5;
    animated_entity_restart(&e, &def);
    if (s->tile != 201 || e.anim.index) return 6;
    return 0;
}

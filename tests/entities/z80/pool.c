#include <zgdk/entities/animated_physics.h>
#include "arena.h"
typedef struct { animated_physics_entity_t entity; uint8_t user[6]; } record;
typedef struct { physics_entity_t entity; uint8_t user[16]; } wide_record;
record records[8];
wide_record wide[3];
entity_pool_t test_pool;
entity_pool_t other_pool;
static const uint8_t frames[] = {201, 42, 255};
static const anim_def def = {frames, 3, 1, ANIM_LOOP};
int main(void) {
    record *a, *b;
    wide_record *w;
    gfx_sprite *sprite;
    if (!reserve()) return 1;
    if (sprite_entities_init(&test_pool, records, 8, sizeof(record))) return 2;
    a = physics_entities_spawn(&test_pool, 0, 20, 30, 8, 9);
    b = physics_entities_spawn(&test_pool, 1, 40, 50, 8, 9);
    if (!a || !b) return 3;
    sprite = a->entity.physics.visual.sprite;
    animated_physics_entity_play(&a->entity, &def);
    sprite->tile = anim_tick(&a->entity.anim);
    physics_entities_sync(&test_pool);
    if (sprite->tile != 201 || sprite->x != 36 || sprite->y != 46) return 4;
    physics_entities_shift(&test_pool, -3, 4);
    physics_entities_sync(&test_pool);
    if (sprite->x != 33 || sprite->y != 50) return 5;
    entity_pool_free(&test_pool, a);
    physics_entities_sync(&test_pool);
    if (sprite->y || sprite->options) return 6;
    a = physics_entities_spawn(&test_pool, 2, 1, 2, 1, 1);
    if (a != records || a->entity.physics.visual.sprite != sprite || sprite->tile) return 7;
    if (sprite_entities_init(&other_pool, wide, 3, sizeof(wide_record))) return 8;
    w = physics_entities_spawn(&other_pool, 0, 100, 100, 1, 1);
    w = physics_entities_spawn(&other_pool, 1, 200, 100, 1, 1);
    if (!w) return 9;
    w->entity.visual.sprite->tile = 42;
    physics_entities_sync(&other_pool);
    if (w->entity.visual.sprite->x != 216 || w->entity.visual.sprite->tile != 42) return 10;
    entity_pool_clear(&other_pool);
    physics_entities_sync(&other_pool);
    if (w->entity.visual.sprite->y) return 11;
    {
        sprite_entity_t small[1];
        entity_pool_t incompatible;
        if (sprite_entities_init(&incompatible, small, 1, sizeof(*small))) return 12;
        if (!sprite_entities_spawn(&incompatible, 0)) return 13;
        small[0].sprite->y = 99;
        physics_entities_sync(&incompatible);
        if (small[0].sprite->y != 99) return 14;
    }
    physics_entities_sync(NULL);
    return 0;
}

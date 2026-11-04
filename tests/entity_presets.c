#include <zgdk/entities/presets.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static gfx_sprite arena[16];
static unsigned used;
void *mem_set(void *p, uint8_t value, size_t n) { return memset(p, value, n); }
gfx_sprite *sprites_register(gfx_sprite sprite) {
    if (used == 16) return NULL;
    arena[used] = sprite;
    return &arena[used++];
}
typedef struct { animated_physics_entity_t entity; uint8_t user[6]; } actor;
int main(void) {
    gfx_sprite gfx;
    sprite_entity_t sprite;
    animated_entity_t animated;
    physics_entity_t physical;
    animated_physics_entity_t both;
    static const uint8_t frames[] = {201, 42, 255};
    static const anim_def loop = {frames, 3, 1, ANIM_LOOP};
    static const anim_def once = {frames, 3, 1, 0};
    actor records[8], second[2], saved[8];
    entity_pool_t pool, other, snapshot;
    actor *a, *b;
    gfx_sprite *reserved;
    unsigned i;
    assert(sprite_entity_init(NULL, &gfx) == ERR_INVALID_PARAMETER);
    assert(sprite_entity_init(&sprite, NULL) == ERR_INVALID_PARAMETER);
    assert(sprite_entity_init(&sprite, &gfx) == ERR_SUCCESS);
    assert(sprite.sprite == &gfx && sprite.parked && sprite.type == 0);
    assert(!gfx.tile && !gfx.y && !gfx.options);
    assert(animated_entity_init(&animated, &gfx) == ERR_SUCCESS);
    animated_entity_play(&animated, &loop);
    assert(gfx.tile == 201);
    gfx.tile = anim_tick(&animated.anim);
    assert(gfx.tile == 201 && animated.anim.index == 1);
    animated_entity_play(&animated, &loop);
    assert(gfx.tile == 201 && animated.anim.index == 1);
    animated_entity_restart(&animated, &loop);
    assert(gfx.tile == 201 && !animated.anim.index);
    animated_entity_play(&animated, NULL);
    assert(!gfx.tile && !animated.anim.def);
    memset(&physical, 0x5a, sizeof(physical));
    assert(physics_entity_init(&physical, &gfx, 1, 2, 0, 8) == ERR_INVALID_PARAMETER);
    assert(physics_entity_init(&physical, &gfx, 1, 2, 8, 9) == ERR_SUCCESS);
    assert(physical.body.x == PHYSICS_FP(1) && physical.body.y == PHYSICS_FP(2));
    assert(physical.body.w == 8 && physical.body.h == 9 && !physical.body.vx);
    assert(animated_physics_entity_init(&both, &gfx, 3, 4, 8, 9) == ERR_SUCCESS);
    animated_physics_entity_play(&both, &once);
    for (i = 0; i < 3; i++) both.physics.visual.sprite->tile = anim_tick(&both.anim);
    assert(both.anim.done && gfx.tile == 255);
    animated_physics_entity_play(&both, &once);
    assert(both.anim.done && gfx.tile == 255);
    animated_physics_entity_restart(&both, &once);
    assert(!both.anim.done && gfx.tile == 201);
    assert(sprite_entities_init(&pool, records, 8, sizeof(*records)) == ERR_SUCCESS);
    assert(physics_entities_compatible(&pool));
    for (i = 0; i < 8; i++) {
        assert(records[i].entity.physics.visual.type == ENTITY_FREE);
        assert(records[i].entity.physics.visual.parked);
    }
    assert(!physics_entities_spawn(&pool, 1, 1, 2, 0, 8));
    assert(!physics_entities_spawn(&pool, ENTITY_FREE, 1, 2, 8, 8));
    a = physics_entities_spawn(&pool, 1, 10, 20, 8, 9);
    reserved = a->entity.physics.visual.sprite;
    animated_physics_entity_play(&a->entity, &loop);
    reserved->tile = anim_tick(&a->entity.anim);
    assert(reserved->tile == 201 && a->entity.anim.index == 1);
    animated_physics_entity_play(&a->entity, &loop);
    assert(reserved->tile == 201);
    a->user[0] = 123;
    b = physics_entities_spawn(&pool, 2, 100, 200, 4, 5);
    entity_pool_free(&pool, b);
    physics_entities_shift(&pool, -3, 4);
    assert(a->entity.physics.body.x == PHYSICS_FP(7) && a->entity.physics.body.y == PHYSICS_FP(24));
    assert(b->entity.physics.body.x == PHYSICS_FP(100));
    entity_pool_clear(&pool);
    assert(pool.used_end == (uint8_t *)(records + 2));
    a = physics_entities_spawn(&pool, 3, 1, 2, 8, 9);
    assert(a == records && a->entity.physics.visual.sprite == reserved);
    assert(!reserved->tile && !a->entity.anim.def && !a->user[0]);
    for (i = 1; i < 8; i++) assert(physics_entities_spawn(&pool, 1, 0, 0, 1, 1) == records + i);
    assert(!physics_entities_spawn(&pool, 1, 0, 0, 1, 1));
    assert(sprite_entities_init(&other, second, 2, sizeof(*second)) == ERR_SUCCESS);
    entity_pool_clear(&other);
    assert(records[0].entity.physics.visual.type == 3);
    snapshot = pool;
    memcpy(saved, records, sizeof(records));
    assert(sprite_entities_init(&pool, records, 0, sizeof(*records)) == ERR_INVALID_PARAMETER);
    assert(sprite_entities_init(&pool, records, 1, sizeof(sprite_entity_t)-1) == ERR_INVALID_PARAMETER);
    assert(sprite_entities_init(&pool, records, 256, sizeof(*records)) == ERR_INVALID_PARAMETER);
    assert(sprite_entities_init(&pool, records, 1, 256) == ERR_INVALID_PARAMETER);
    assert(!memcmp(&snapshot, &pool, sizeof(pool)) && !memcmp(saved, records, sizeof(records)));
    {
        sprite_entity_t small[2];
        assert(sprite_entities_init(&other, small, 2, sizeof(*small)) == ERR_SUCCESS);
        assert(!physics_entities_compatible(&other));
        assert(!physics_entities_spawn(&other, 1, 0, 0, 1, 1));
        assert(sprite_entities_spawn(&other, 0) == small);
        physics_entities_shift(&other, 1, 1);
        assert(small[0].type == 0);
    }
    used = 15; /* One reservation succeeds before the second fails. */
    assert(sprite_entities_init(&other, second, 2, sizeof(*second)) == ERR_NO_MORE_MEMORY);
    assert(!other.storage && !other.used_end && !sprite_entities_spawn(&other, 0));
    assert(used == 16 && !arena[15].y && !arena[15].options);
    used = 0;
    assert(sprite_entities_init(&other, second, 2, sizeof(*second)) == ERR_SUCCESS);
    puts("entity presets: all four types, retained frames, pool lifecycle and rebasing passed");
    return 0;
}

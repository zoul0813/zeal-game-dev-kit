#include <zgdk/entities/physics.h>
void physics_entities_shift(entity_pool_t *pool, int16_t dx, int16_t dy) {
    uint8_t *p;
    int16_t fx = PHYSICS_FP(dx), fy = PHYSICS_FP(dy);
    if (!physics_entities_compatible(pool)) return;
    for (p = pool->storage; p < pool->used_end; p += pool->stride) {
        physics_entity_t *entity = (void *)p;
        if (entity->visual.type == ENTITY_FREE) continue;
        entity->body.x += fx;
        entity->body.y += fy;
    }
}

#include <zgdk/entities/physics.h>
void *physics_entities_spawn(entity_pool_t *pool, uint8_t type,
    int16_t px, int16_t py, uint8_t w, uint8_t h) {
    physics_entity_t *entity;
    if (!physics_entities_compatible(pool) || !w || !h) return NULL;
    entity = sprite_entities_spawn(pool, type);
    if (entity) body_init(&entity->body, px, py, w, h);
    return entity;
}

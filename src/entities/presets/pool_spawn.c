#include <zgdk/entities/sprite.h>
#include <core.h>
void *sprite_entities_spawn(entity_pool_t *pool, uint8_t type) {
    sprite_entity_t *entity;
    gfx_sprite *sprite;
    if (!pool || !pool->storage || pool->stride < sizeof(sprite_entity_t)
        || pool->tag_offset != offsetof(sprite_entity_t, type)) return NULL;
    entity = entity_pool_alloc(pool, type);
    if (!entity) return NULL;
    sprite = entity->sprite;
    mem_set(entity, 0, pool->stride);
    entity->sprite = sprite;
    entity->type = type;
    sprite->tile = 0;
    return entity;
}

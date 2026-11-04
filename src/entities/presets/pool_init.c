#include <zgdk/entities/sprite.h>
#include <zgdk/sprites.h>
#include <core.h>
zos_err_t sprite_entities_init(entity_pool_t *pool, void *records, uint16_t count, uint16_t stride) {
    uint8_t *p;
    uint8_t n;
    zos_err_t err;
    gfx_sprite hidden = {0};
    if (stride < sizeof(sprite_entity_t)) return ERR_INVALID_PARAMETER;
    err = entity_pool_init(pool, records, count, stride, offsetof(sprite_entity_t, type));
    if (err != ERR_SUCCESS) return err;
    p = records;
    n = count;
    do {
        sprite_entity_t *entity = (void *)p;
        mem_set(p, 0, stride);
        entity->type = ENTITY_FREE;
        entity->parked = 1;
        entity->sprite = sprites_register(hidden);
        if (!entity->sprite) {
            pool->storage = NULL;
            pool->used_end = NULL;
            return ERR_NO_MORE_MEMORY;
        }
        if (--n) p += stride;
    } while (n);
    return ERR_SUCCESS;
}

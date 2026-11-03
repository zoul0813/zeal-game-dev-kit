#include "zgdk/entities.h"

void *entity_pool_alloc(entity_pool_t *pool, uint8_t type) {
    uint8_t *tag;
    uint8_t n;
    if (type == ENTITY_FREE) return NULL;
    tag = pool->storage + pool->tag_offset;
    n = pool->capacity;
    while (*tag != ENTITY_FREE) {
        if (!--n) return NULL;
        tag += pool->stride;
    }
    *tag = type;
    tag -= pool->tag_offset;
    if (tag >= pool->used_end) pool->used_end = tag + pool->stride;
    return tag;
}

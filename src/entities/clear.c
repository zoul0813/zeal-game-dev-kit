#include "zgdk/entities.h"

void entity_pool_clear(entity_pool_t *pool) {
    uint8_t *entry;
    for (entry = pool->storage; entry < pool->used_end; entry += pool->stride)
        entry[pool->tag_offset] = ENTITY_FREE;
}

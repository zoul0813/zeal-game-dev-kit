#include "zgdk/entities.h"

void entity_pool_free(entity_pool_t *pool, void *entry) {
    if (entry) ((uint8_t *)entry)[pool->tag_offset] = ENTITY_FREE;
}

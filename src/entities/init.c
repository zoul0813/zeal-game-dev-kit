#include "zgdk/entities.h"

zos_err_t entity_pool_init(entity_pool_t *pool, void *storage, uint16_t count,
                          uint16_t stride, uint16_t tag_offset) {
    uint8_t *tag;
    uint8_t n;
    if (!pool || !storage || !count || count > 255 || !stride || stride > 255 || tag_offset >= stride)
        return ERR_INVALID_PARAMETER;
    pool->storage = storage;
    pool->used_end = storage;
    pool->capacity = count;
    pool->stride = stride;
    pool->tag_offset = tag_offset;
    tag = pool->storage + tag_offset;
    n = count;
    do {
        *tag = ENTITY_FREE;
        if (--n) tag += stride;
    } while (n);
    return ERR_SUCCESS;
}

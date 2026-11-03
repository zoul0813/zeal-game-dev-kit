#ifndef ZGDK_ENTITIES_H
#define ZGDK_ENTITIES_H

#include <stdint.h>
#include <stddef.h>
#include <zos_errors.h>

#define ENTITY_FREE 0xff

/* Caller-owned records; only the tag byte belongs to the pool. No heap, graphics,
 * physics or animation dependencies. Treat initialized descriptor fields as read-only.
 * used_end is a high-water mark, retained by free/clear for deferred cleanup. */
typedef struct {
    uint8_t *storage;
    uint8_t *used_end;
    uint8_t capacity;
    uint8_t stride;
    uint8_t tag_offset;
} entity_pool_t;

/* storage holds count records of stride bytes, with a uint8_t tag at tag_offset.
 * count/stride must be 1..255; tag_offset < stride. Invalid input leaves both
 * descriptor and records unchanged. Valid initialization changes only tag bytes.
 * Storage must remain alive and fit in addressable memory, without wrapping. */
zos_err_t entity_pool_init(entity_pool_t *pool, void *storage, uint16_t count,
                          uint16_t stride, uint16_t tag_offset);
/* Lowest free slot; NULL if full or type == ENTITY_FREE. Sets only the tag;
 * payload survives reuse and is initialized by the caller. */
void *entity_pool_alloc(entity_pool_t *pool, uint8_t type);
/* entry must be a record belonging to this initialized pool (NULL is allowed).
 * Free/clear preserve payload and used_end; neither performs resource cleanup. */
void entity_pool_free(entity_pool_t *pool, void *entry);
void entity_pool_clear(entity_pool_t *pool);
/* Typed traversal: entry is an existing typed pointer, tag is the uint8_t member
 * passed to init via offsetof. Use a side-effect-free pool pointer. The compiler
 * knows record size/tag offset; no per-record iterator call or dynamic stride.
 * Freeing records, break and continue are supported. Later slots allocated during
 * traversal are visited; earlier reused slots are not revisited. Do not reinitialize
 * the pool while iterating. entry is one-past-used on normal completion.
 * Always follow the macro with a braced loop body. */
#define ENTITY_POOL_FOR_EACH(pool, entry, tag) \
    for ((entry) = (void *)(pool)->storage; \
         (uint8_t *)(entry) < (pool)->used_end; ++(entry)) \
        if ((entry)->tag == ENTITY_FREE) {} else

#endif

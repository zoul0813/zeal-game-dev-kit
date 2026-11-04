#ifndef ZGDK_ENTITIES_PHYSICS_H
#define ZGDK_ENTITIES_PHYSICS_H
#include <zgdk/entities/sprite.h>
#include <zgdk/physics/body.h>
typedef struct { sprite_entity_t visual; body_t body; } physics_entity_t;
#ifdef __SDCC
_Static_assert(sizeof(physics_entity_t) == 21 && offsetof(physics_entity_t, body) == 9, "physics entity ASM layout");
#endif
/* Structural check for an initialized, read-only preset pool descriptor. Caller
 * still owns the C record contract: a physical preset must be its first member. */
INLINE uint8_t physics_entities_compatible(const entity_pool_t *pool) {
    return pool && pool->storage && pool->stride >= sizeof(physics_entity_t)
        && pool->tag_offset == offsetof(sprite_entity_t, type);
}
zos_err_t physics_entity_init(physics_entity_t *entity, gfx_sprite *sprite,
    int16_t px, int16_t py, uint8_t w, uint8_t h);
/* Works for physical and animated-physical records. Full record is cleared, then
 * body initialized; no gravity/controller/collision step runs implicitly. */
void *physics_entities_spawn(entity_pool_t *pool, uint8_t type,
    int16_t px, int16_t py, uint8_t w, uint8_t h);
void physics_entity_sync(physics_entity_t *entity);
/* Optimized ASM batch: skips free/hidden records and parks each sprite once.
 * Uses actual pool stride/high-water mark. Shared view/scratch make sync
 * non-reentrant: do not call from an interrupt or nest sync calls. */
void physics_entities_sync(const entity_pool_t *pool);
/* Shift live bodies by integer pixels. Keep offsets and resulting fixed-point
 * positions representable, as for body_t arithmetic. Incompatible pools no-op. */
void physics_entities_shift(entity_pool_t *pool, int16_t dx, int16_t dy);
#endif

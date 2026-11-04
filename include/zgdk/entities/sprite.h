#ifndef ZGDK_ENTITIES_SPRITE_H
#define ZGDK_ENTITIES_SPRITE_H
#include <zgdk/entities.h>
#include <zgdk/inline.h>
#include <zvb_sprite.h>

/* A reserved arena sprite plus its world-rendering properties. type is the
 * game-defined allocation tag (ENTITY_FREE is reserved). No position duplicate:
 * standalone sync receives a position; physical presets own one body position. */
typedef struct {
    gfx_sprite *sprite;
    uint8_t type;
    uint8_t flip, tall, hidden;
    int8_t ox, oy;
    uint8_t parked; /* renderer-owned: already hidden in hardware */
} sprite_entity_t;
#ifdef __SDCC
_Static_assert(sizeof(sprite_entity_t) == 9, "sprite entity ASM layout");
_Static_assert(offsetof(sprite_entity_t, type) == 2, "sprite entity ASM tag");
_Static_assert(offsetof(sprite_entity_t, parked) == 8, "sprite entity ASM flags");
_Static_assert(offsetof(gfx_sprite, options) == 6 && sizeof(gfx_sprite) == 8, "hardware sprite ASM layout");
#endif

/* Bind caller-owned, live arena storage; reset rendering state and hide sprite.
 * No reservation is made by this standalone initializer. */
zos_err_t sprite_entity_init(sprite_entity_t *entity, gfx_sprite *sprite);
/* Records must begin with one of these presets, including through nested first
 * members. count/stride have the pool's 1..255 limits; stride >= preset size.
 * Reserve count sprites after sprites_register_arena. On arena exhaustion the
 * pool is invalidated and reservations already made remain hidden: reset/re-register
 * the arena before retrying. Invalid parameters leave records/descriptor unchanged. */
zos_err_t sprite_entities_init(entity_pool_t *pool, void *records, uint16_t count, uint16_t stride);
/* Clear the entire caller record while preserving its permanent sprite binding.
 * Allocates lowest free slot; NULL on full pool, invalid layout or reserved tag. */
void *sprite_entities_spawn(entity_pool_t *pool, uint8_t type);
/* Active camera for preset sync. Default 0,0. Set before syncing a different view.
 * Coordinates must fit signed 12.4: -2048..2047 pixels. No implicit camera follow. */
void entity_view_set(int16_t x, int16_t y);
/* Culling size, default 320x240. Supports graphics up to 640x480.
 * Invalid dimensions leave the current bounds unchanged. */
zos_err_t entity_view_size(uint16_t width, uint16_t height);
/* Fixed-point entry is ASM-only. Retained tile is never recomputed or overwritten.
 * Signed pixel offsets apply before culling. Hardware can display screen Y=-16
 * only for tall sprites; positions above -16 are parked for both heights.
 * Arena pointers must remain valid; re-registering/deregistering invalidates them. */
void sprite_entity_sync_fp(sprite_entity_t *entity, int16_t x, int16_t y);
INLINE void sprite_entity_sync(sprite_entity_t *entity, int16_t x, int16_t y) {
    sprite_entity_sync_fp(entity, (int16_t)(x * 16), (int16_t)(y * 16));
}
#endif

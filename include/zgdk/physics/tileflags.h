#include <stdint.h>
#include <stddef.h>
#include <zgdk/inline.h>

#ifndef ZGDK_PHYSICS_TILEFLAGS_H
#define ZGDK_PHYSICS_TILEFLAGS_H

/**
 * Per-tile behaviour flags, looked up by tile index in a 256-entry table.
 * Collision reads the tile index from the registered ZGDK Tilemap.
 */
#define TF_NONE    0x00
#define TF_SOLID   0x01  // blocks from every side
#define TF_ONEWAY  0x02  // blocks only when landing on it from above
#define TF_LADDER  0x04  // climbable
#define TF_HAZARD  0x08  // hurts on touch
#define TF_USER1   0x10  // free for the game (sockets, doors, ...)
#define TF_USER2   0x20
#define TF_USER3   0x40
#define TF_USER4   0x80

/**
 * @brief Register the flags table (256 entries, indexed by tile)
 */
void tileflags_register(const uint8_t *table);

/**
 * @brief Flags returned for points outside the tilemap (default TF_SOLID)
 *
 * Flip-screen games usually set TF_NONE so bodies can walk off an edge.
 */
void tileflags_set_outside(uint8_t flags);

/**
 * @brief Flags of the tile containing tilemap pixel `px`, `py`
 */
uint8_t tileflags_at(int16_t px, int16_t py);

/**
 * @brief Flags of the tile at tile coordinate `tx`, `ty`
 */
uint8_t tileflags_at_tile(uint8_t tx, uint8_t ty);

/**
 * @brief Re-read the registered ZGDK Tilemap into the lookup cache
 *
 * Call after a successful
 * tilemap_load() or your own tilemap_register(), before using tileflags_fast().
 */
void tileflags_refresh(void);

/* One caller-owned row cache per application. Define in exactly one C file,
 * sized to the tallest logical map. No fixed SDK allocation or pointer lookup.
 * Changing a map's dimensions in place requires tileflags_refresh(). */
#define TILEFLAGS_DEFINE_CACHE(height) \
    _Static_assert((height) > 0 && (height) <= 255, "tileflags cache height must be 1..255"); \
    const uint8_t *tileflags_rows[(height)]; \
    const uint8_t tileflags_row_capacity = (height)
extern const uint8_t tileflags_row_capacity;

/* lookup cache, filled by tileflags_refresh() */
extern const uint8_t *_tf_flags;
extern const uint8_t *tileflags_rows[];

/**
 * @brief Bare lookup for hot paths: no bounds or cache checks
 *
 * Only for in-map pixel coordinates after tileflags_refresh(); use tileflags_at() otherwise.
 */
INLINE uint8_t tileflags_fast(uint16_t px, uint16_t py) {
    return _tf_flags[tileflags_rows[py >> 4][px >> 4]];
}

#endif

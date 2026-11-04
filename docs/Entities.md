# Entity pools

Include `<zgdk/entities.h>`. A pool allocates caller-defined fixed-size records.
It has no physics, animation, sprite, tilemap or heap dependency. Independent
pools can contain unrelated record types, including invisible objects or tasks.

For optional sprite, animated, physical and animated-physical record types, see
[Entity presets](EntityPresets.md). These do not change the generic pool contract.

```c
#include <zgdk/entities.h>

typedef struct {
    uint16_t score;
    uint8_t kind;  /* game-defined tag, 0xff means free */
    uint8_t ticks;
} pickup_t;

static pickup_t pickups[16];
static entity_pool_t pool;

/* Check the result before using the pool. */
zos_err_t init_pickups(void) {
    return entity_pool_init(&pool, pickups, 16, sizeof(pickup_t),
                            offsetof(pickup_t, kind));
}

void spawn_pickup(void) {
    pickup_t *p = entity_pool_alloc(&pool, 1);
    if (!p) return;
    p->score = 100;
    p->ticks = 0;
}

void update_pickups(void) {
    pickup_t *p;
    ENTITY_POOL_FOR_EACH(&pool, p, kind) {
        if (++p->ticks == 60) entity_pool_free(&pool, p);
    }
}
```

The tag must be a `uint8_t` field anywhere inside the record. All values except
`ENTITY_FREE` (255) are application-defined. Count and record size must each be
1–255, and the tag offset must be smaller than record size. Initialization
rejects invalid parameters without changing the descriptor or storage. Its
16-bit arguments avoid silently truncating invalid `sizeof`/`offsetof` values.
The caller provides suitably aligned, writable storage for all records; the
buffer and its end must fit in addressable memory without wrapping. Keep it
alive until the pool is no longer used. A descriptor occupies seven bytes on Z80.

Initialization marks every tag free; allocation takes the lowest free slot and
sets its tag. Neither clears payload bytes. Initialize your fields after
allocation, or retain permanent resources across reuse. Full pools and an
allocation with the reserved tag return NULL. There are no callbacks or implicit
resource destructors. Use only successfully initialized descriptors.

Free accepts NULL; any other pointer must be a record from that pool. Clear
marks all used slots free. Neither operation changes payload or the `used_end`
high-water pointer. That pointer lets a renderer clean up previously visible
objects after free/clear. Reinitialization resets the high-water mark.

`ENTITY_POOL_FOR_EACH(&pool, pointer, tag_member)` visits active records in
slot order and stops at that mark. Declare the typed pointer first, use the same
tag member supplied to initialization, and always use a braced body. The macro
uses compile-time record size and tag offsets, avoiding dynamic-stride calls in
hot game loops. Pass a side-effect-free pool pointer. The iterator ends one past
the used range; break leaves it at the current record. Freeing the current
record during traversal is supported. Later slots allocated during traversal
are visited; reused slots already passed are not revisited. Do not reinitialize
the pool while iterating. Freed pointers have no generation protection.
Descriptor fields are public for direct typed traversal and cleanup, but callers
must treat them as read-only.

Each public function occupies a separate archive member. Including this header
or linking a pool does not link physics or graphics. Optional bodies, controllers,
animations and render resources belong in the application's own record type.

Run `sh tests/run-entities.sh` for the ASan/UBSan lifecycle tests. They cover
non-leading tags, 255 slots, independent pools, exhaustion, reuse, retained data,
iteration, clear/high-water behavior, reinitialization and invalid configuration.

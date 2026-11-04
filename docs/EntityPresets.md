# Optional entity presets

The generic `<zgdk/entities.h>` pool still accepts arbitrary records without
physics, animation or graphics. These opt-in presets compose common components:

| Header | Type | Contents | Z80 bytes |
|---|---|---|---:|
| `entities/sprite.h` | `sprite_entity_t` | Reserved sprite, allocation tag, rendering properties | 9 |
| `entities/animated.h` | `animated_entity_t` | `visual` sprite entity + `anim` | 14 |
| `entities/physics.h` | `physics_entity_t` | `visual` sprite entity + `body` | 21 |
| `entities/animated_physics.h` | `animated_physics_entity_t` | `physics` entity + `anim` | 26 |

`<zgdk/entities/presets.h>` includes all four; `<zgdk.h>` includes that umbrella.
Individual headers and archive members keep unused capabilities out of the
executable. Non-animated physics saves five bytes per record. Sizes above exclude
the arena's eight-byte `gfx_sprite` packet and application fields.

The `visual` prefix stores `sprite`, game-defined `type`, `flip`, `tall`, `hidden`,
pixel offsets `ox/oy`, and renderer-owned `parked`. Type values 0–254 belong to the
application; `ENTITY_FREE` (255) is reserved for unused slots. Flip uses the ZVB
flip flags; tall selects a 16x32 sprite. The renderer preserves palette/priority
flags and the retained tile, and parks free/hidden/offscreen sprites once by
clearing Y and options. World coordinates come from the caller or the body, not
from a duplicate position in the visual component.

Offsets are applied to signed screen coordinates before culling, so they can
bring an otherwise offscreen origin into view. Hardware Y is an unsigned
screen coordinate plus 16: a tall sprite at screen Y=-16 displays its bottom
tile, while a normal sprite is fully hidden there. Positions above Y=-16 cannot
be represented by the hardware sprite packet and are parked for both heights.

## Standalone entities

Reserve a live arena sprite with the existing `sprites_register_arena()` and
`sprites_register()` APIs, then initialize the chosen preset:

```c
physics_entity_t crate;
/* slot is a successfully reserved gfx_sprite*. Check every result. */
zos_err_t err = physics_entity_init(&crate, slot, 40, 80, 16, 16);
if (err != ERR_SUCCESS) return err;
crate.visual.sprite->tile = CRATE_TILE;

/* Physics is explicit: no automatic gravity, collision or controller. */
body_accelerate(&crate.body, 1, 0);
body_integrate(&crate.body);
entity_view_set(camera_x, camera_y);
physics_entity_sync(&crate);
```

Initializers are `sprite_entity_init`, `animated_entity_init`,
`physics_entity_init`, and `animated_physics_entity_init`. They reset the relevant
components and hide the bound sprite. Invalid pointers and zero physical body
sizes are rejected before mutation. Sprite binding does not reserve arena storage.
The caller must keep that storage alive. Deregistering or re-registering an arena
invalidates bindings; reinitialize the entities before using them again.

Use `sprite_entity_sync(&entity.visual, x, y)` for animated sprites, or
`sprite_entity_sync(&entity, x, y)` for plain sprites. These coordinates are integer
pixels. `sprite_entity_sync_fp` is the underlying ASM fixed-point entry. Physical
and animated-physical entities share `physics_entity_sync(&entity.physics)` (or
`&entity` for non-animated physics). Body positions are signed 12.4 fixed-point;
keep world and camera coordinates within -2048..2047 pixels and arithmetic sums
representable, as required by `body_t`.

The active view defaults to camera 0,0 and a 320x240 screen. `entity_view_set(x,y)`
changes the camera. `entity_view_size(width,height)` changes software culling
bounds, supporting sizes 1..640 by 1..480 for the ZVB graphics modes. Invalid
sizes leave bounds unchanged. This is culling, not a hardware scissor rectangle.
Set the correct view before syncing a different camera or graphics size.

## Animation and retained tiles

`animated_entity_play/restart` and `animated_physics_entity_play/restart` combine
playback selection with seeding the first sprite tile, without consuming a tick.
Selecting the same definition preserves both timing and the last returned tile.
Restart always resets both; NULL clears playback and tile.

```c
animated_physics_entity_play(&enemy->entity, &walk);
enemy->entity.physics.visual.sprite->tile = anim_tick(&enemy->entity.anim);
```

Tick returns this update's tile before preparing the next index. The ASM renderer
leaves that tile intact. Skipping a tick holds the displayed tile; do not overwrite
it by peeking at the advanced `anim.index`. Animation never steps physics, and
physics never ticks animation. See [Animations](Anims.md).

## Pools with game-owned records

Application records embed a preset as their **first member**, optionally through
nested first members. This is real C composition, not a cast between unrelated
structures. The common sprite prefix remains at byte zero. Application fields
follow the preset; no callbacks, component registry or allocation heap are used.

```c
typedef struct {
    animated_physics_entity_t entity;
    uint8_t health;
} enemy_t;

static enemy_t records[16];
static entity_pool_t enemies;

/* After registering an arena with enough available slots: */
zos_err_t err = sprite_entities_init(&enemies, records, 16, sizeof(*records));
if (err != ERR_SUCCESS) return err;

enemy_t *enemy = physics_entities_spawn(&enemies, ENEMY, 40, 80, 12, 14);
if (enemy) {
    enemy->health = 3;
    animated_physics_entity_play(&enemy->entity, &walk);
}

ENTITY_POOL_FOR_EACH(&enemies, enemy, entity.physics.visual.type) {
    /* Explicit body/controller updates belong here. */
    enemy->entity.physics.visual.sprite->tile = anim_tick(&enemy->entity.anim);
}
physics_entities_sync(&enemies);
```

`sprite_entities_init` validates the generic pool dimensions, clears the records,
marks their prefix tags free and reserves one permanent sprite per slot. Count
and stride must each be 1–255; stride must cover the whole selected preset and
record. Prefix tags are fixed at `offsetof(sprite_entity_t, type)` for fast batch
rendering. Use `entity_pool_free/clear` directly; high-water cleanup is retained.

`sprite_entities_spawn` takes a free slot, clears its entire application record,
restores its permanent binding and clears the retained tile. Use it for sprite or
animated records. `physics_entities_spawn` additionally initializes the shared
physical prefix, so it serves both physical variants. It rejects zero body sizes
and incompatible pool layouts. Both return NULL on exhaustion or reserved type.
Fields such as health, timers, platformer controller state or user bytes remain
application-owned and are initialized by the application after spawning.

If sprite reservation fails, initialization returns ERR_NO_MORE_MEMORY and
invalidates the preset pool. Reservations already made remain hidden and held;
reset/re-register the arena before retrying. This does not silently deregister
other owners. Invalid configuration leaves the descriptor and records unchanged.
Only use successfully initialized preset pools, and never overwrite their
read-only descriptor fields. `physics_entities_compatible` checks the layout of
an initialized descriptor; it is not an ownership/generation validator.

`physics_entities_shift(pool, dx, dy)` rebases active body positions in pixels;
free slots and animation/controller state remain unchanged. Keep resulting
fixed-point coordinates representable. For animated-physical records it uses
the same shared physical prefix, with no animation dependency in the shift code.

## Rendering, ownership and tests

Standalone and batch synchronization use one shared ASM source body, assembled
into separate archive capabilities. There is no C rendering alternative or debug
flag. The batch path uses the pool's actual stride and high-water pointer, skips
free/hidden records and avoids per-record function calls. Applications using only
batch rendering do not link standalone renderers. Global view state and batch
scratch make synchronization non-reentrant; do not invoke or nest it from an
interrupt. The application owns frame scheduling and arena lifetime.

`sh tests/run-entity-presets.sh` runs native ASan/UBSan lifecycle/animation checks
with ZOS_PATH, ZVB_SDK_PATH and COREUTILS_PATH set. No native renderer is substituted.
`sh tests/run-entity-presets-z80.sh` requires a rebuilt library, SDCC tools, those
dependencies, and Python with `z80` installed (`PYTHON` may select its environment).
It tests all four families using real arena registration and actual ASM, verifies
unused capabilities are absent from linker maps, tests variable record strides,
640x480 culling, retained frames, cleanup and incompatible-pool rejection.
The checker can also inspect a game `.ihx` with adjacent `.map`:
`python tests/entities/z80/preset_check.py path/to/game.ihx [baseline.ihx]`.

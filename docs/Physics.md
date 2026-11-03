# Physics

Use `<zgdk/physics/body.h>` for free bodies, `<zgdk/physics/tilemap.h>` for
optional tile collision, and `<zgdk/physics/platformer.h>` for the optional
platformer controller. `<zgdk/physics.h>` includes the generic body/tile APIs;
it does not include the platformer controller.

Each capability has its own archive member. Free bodies link no tile flags,
row cache or controller code. No heap allocation or sprite is required.

## Bodies and units

`body_t` is a 12-byte rectangle on Z80: signed 16-bit `x`, `y`, `vx`, `vy`,
then byte-sized `w`, `h`, `state`, and `touched`. Position is the hitbox's
top-left. Width and height are nonzero pixels (1..255).

Positions, velocities and accelerations use 12.4 fixed point: one pixel is 16
units. `PHYSICS_FP(10)` converts pixels to fixed point; `PHYSICS_FP_INT(value)`
converts back. Speeds/accelerations are per game tick, not per second. Keep
positions, velocities and sums within signed 16-bit range; rebase long worlds.

A shooter can use only the body header:

```c
#include <zgdk/physics/body.h>

body_t ship;
body_init(&ship, 100, 100, 12, 12);
/* Each tick: fractional thrust in either axis, then free movement. */
body_accelerate(&ship, 1, -1);
body_integrate(&ship);
```

`body_accelerate` adds to velocity: apply once for an impulse, or each tick for
acceleration. The game supplies speed limits and input policy.
`body_apply_gravity(body, gravity, max_fall)` adds downward acceleration and
caps downward velocity at a positive terminal speed. These small operations
are `INLINE`, avoiding forwarding calls and unused SDCC helper copies.

`body_integrate` advances both coordinates without consulting a map or changing
collision state. `body_overlap` tests rectangular overlap. `body_set_pos`
teleports in integer pixels, preserving velocity and clearing collision/policy
state. Bodies contain no mass, animation, sprite, health or jump timers.

## Tile flags and row storage

The caller owns a 256-byte table mapping tile IDs to byte flags. Flags classify
tile IDs, not individual map cells. The registered ZGDK Tilemap supplies each
cell's tile ID. There is no second map-sized collision buffer.

Define the row cache in exactly one C file:

```c
#include <zgdk/physics/tilemap.h>
TILEFLAGS_DEFINE_CACHE(15);   /* tallest logical map used by this application */
```

Logical map widths must fit a byte (1..255 tiles).

The macro defines `tileflags_rows` and `tileflags_row_capacity`. On Z80 the
row pointers cost two bytes per row (30 bytes for 15 rows), plus one byte for
the capacity constant. Caller-owned storage preserves compact 15-row games and
also permits tall logical maps, without rebuilding `zgdk.lib`. The fixed symbol
allows direct row indexing in the optimized Z80 probes, without another pointer
indirection. Cache capacity must be 1..255; rows beyond capacity are outside.

Register the game's table with `tileflags_register(flags)`. After successfully
loading/registering a map, call `tileflags_refresh()` before fast lookups.
Safe lookups automatically refresh when the registered map or tile buffer
changes. If dimensions change in place, refresh explicitly. The map, tile
buffer and flag table must remain alive while used.

* `tileflags_at(px, py)` and `tileflags_at_tile(tx, ty)` check boundaries.
* `tileflags_set_outside(flags)` sets the outside classification (default solid).
* `tileflags_fast(px, py)` is an unchecked inline lookup: use only in-range
  coordinates after refreshing the cache and registering a valid table.

A lookup returns all eight game-provided bits. Collision interprets `TF_SOLID`
(bit 0) and `TF_ONEWAY` (bit 1). `TF_LADDER` (bit 2) and `TF_HAZARD` (bit 3)
are conventions used by controller/contact helpers; `TF_USER1..4` are game
flags. A pure classification table can use its own eight bits. With collision,
preserve solid/one-way meanings; other contact flags need not imply a platformer.

## Tile collision

`body_step(body)` moves by velocity, resolving X and then Y against the logical
map. It adds no gravity and handles no input. With no map or table registered,
it moves freely. This supports top-down movement, projectiles and swimming.

Solid tiles block all sides. One-way tiles catch downward crossings from above.
`BODY_PASS_ONEWAY` bypasses one-way collision and persists across steps; other
result bits are rebuilt each step:

* `BODY_ON_GROUND`: supported from below (positive Y).
* `BODY_HIT_WALL`: blocked horizontally.
* `BODY_HIT_CEIL`: blocked above (negative Y).
* `BODY_IN_HAZARD`, `BODY_ON_LADDER`: standard contact conveniences.

`touched` reports the masked OR of sampled contact flags. Hazard sampling is
slightly inset, preserving the original forgiving spike corners. Ladder contact
uses the center sample. `body_set_touch_mask(0)` skips contact probes entirely;
solid/one-way collision still works. The default mask is `0xff`.

This is discrete tile collision, not swept collision. Use at most 16 pixels per
axis per step, or substep faster bodies to prevent tunneling. Physics uses the
single registered ZGDK Tilemap and shared probe scratch: it is not reentrant.
Coordinates are room/world-local pixels, independent of camera/VRAM offsets.
The Z80 probes remain hand-written assembly; host builds use the C reference.

## Optional platformer controller

`platformer_state_t` stores three bytes (`coyote`, `buffer`, `air_left`) separately
from a body. Allocate/zero-initialize state only for bodies needing the controller.
`platformer_config_t` supplies gravity, terminal speed, jump speed/cut, running
acceleration/braking/speed, coyote time, jump buffering and extra air jumps.

```c
#include <zgdk/physics/platformer.h>

body_t hero;
platformer_state_t control;
static const platformer_config_t config = {
    .gravity = 4, .max_fall = 96, .jump_vel = -70, .jump_cut = -24,
    .accel = 4, .decel = 6, .max_run = 28,
    .coyote = 5, .buffer = 6, .air_jumps = 1,
};
/* After the map/table/cache have been initialized: */
platformer_init(&hero, &control, 32, 160, 12, 30);
/* Each tick, with directions -1/0/+1 and button states supplied by the game: */
platformer_ladder(&hero, up, down, dir_x, 16);
if (platformer_update(&hero, &config, &control, dir_x, jump_held, jump_pressed)) {
    /* A jump started; play the game's sound/animation. */
}
platformer_move(&hero, &config);
```

`platformer_update` changes velocity/timers and returns whether a jump started.
Coyote time permits a jump shortly after leaving ground; buffering remembers a
jump just before landing. Releasing jump cuts the rise; optional air jumps are
replenished on ground.

Ladders use `TF_LADDER`; a standable ladder lip normally combines it with
`TF_ONEWAY`. While climbing, the controller uses `BODY_PASS_ONEWAY` to allow
lip descent, and `platformer_move` omits gravity. `platformer_is_climbing`
queries this mode. The game can supply horizontal alignment or ladder art.

`platformer_set_pos` combines teleport and controller reset. `platformer_reset`
clears buffered/coyote jumps on a room change, preserving remaining air jumps;
`platformer_init` also initializes those air jumps to zero. Pair each body with
its own controller state. Spawn positions, input reads, damage, enemies,
checkpoints, room transitions and rendering remain game-owned.

## Validation

Run `sh tests/run-physics.sh` for sanitizer-enabled free-body tests followed by
Z80 collision and platformer behavior tests. The free-body test links without a
tilemap, cache definition or platformer implementation, verifying that those are
optional. Tilemap collision functions live in `src/physics/tilemap/`.
Probes and edge calculations use only the assembly implementation in `body_probe_row.asm`,
`body_probe_column.asm` and `body_edges.asm`;
there is no C fallback.

Target checks (after building the library, with SDCC/makebin and SDK environment
variables available): `sh tests/run-physics-target.sh`. Run
`sh tests/run-physics-z80.sh` with Python `z80` installed for randomized probes
against a Python reference, fixed-point edges, and collision/platformer scenarios.
`PYTHON`, `SDCC`, `AS`, `MAKEBIN` and `ZGDK_LIB` can select installed tools
or the library directory for those checks.

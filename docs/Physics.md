# Physics

Use `<zgdk/physics/body.h>` for free bodies, `<zgdk/physics/tilemap.h>` for
optional tile collision, and `<zgdk/physics/platformer.h>` for the optional
platformer controller, or `<zgdk/physics/inertial.h>` for eight-way inertial
movement. `<zgdk/physics.h>` includes the generic body/tile APIs;
it does not include either optional controller.

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

## Tile flags

Tile flags are a general tilemap service, exposed through `<zgdk/tilemap/flags.h>`.
See [Tile flags and row storage](Tilemaps.md#tile-flags-and-row-storage) for
registration, caller-owned cache storage, and lookups. Physics consumes that
same table and cache.

A lookup returns all eight game-provided bits. Collision interprets `TF_SOLID`
(bit 0) and `TF_ONEWAY` (bit 1). `TF_LADDER` (bit 2) and `TF_TRIGGER` (bit 3)
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
* `BODY_TOUCHING_TRIGGER`: contact with a `TF_TRIGGER` tile; the game decides
  the response (checkpoints, pickups, exits, damage, or other interactions).
* `BODY_ON_LADDER`: center contact with a `TF_LADDER` tile.

`touched` reports the masked OR of sampled contact flags. Trigger sampling is
slightly inset to avoid reporting grazed tile corners. Trigger contact is
recomputed each step; the game controls whether an interaction happens once or
continuously. Ladder contact uses the center sample. `body_set_touch_mask(0)` skips contact probes entirely;
solid/one-way collision still works. The default mask is `0xff`.

This is discrete tile collision, not swept collision. Use at most 16 pixels per
axis per step, or substep faster bodies to prevent tunneling. Physics uses the
single registered ZGDK Tilemap and shared probe scratch: it is not reentrant.
Coordinates are room/world-local pixels, independent of camera/VRAM offsets.
The Z80 probes remain hand-written assembly; collision tests run in a Z80 emulator.

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
optional. Physics functions are split by family under `src/physics/body/`,
`tilemap/` and `platformer/`, with one function per source file. General tile-flag
lookups live in `src/tilemap/flags/`.
Probes and edge calculations use only the assembly implementation in `body_probe_row.asm`,
`body_probe_column.asm` and `body_edges.asm`;
there is no C fallback.

Target checks (after building the library, with SDCC/makebin and SDK environment
variables available): `sh tests/run-physics-target.sh`. Run
`sh tests/run-physics-z80.sh` with Python `z80` installed for randomized probes
against a Python reference, fixed-point edges, and collision/platformer scenarios.
`PYTHON`, `SDCC`, `AS`, `MAKEBIN` and `ZGDK_LIB` can select installed tools
or the library directory for those checks.

## Inertial controller

Include `<zgdk/physics/inertial.h>` for optional eight-way acceleration, drag,
speed limits and glide after impulses. It changes velocity only, has no mutable
controller state, and depends only on `body_t`. It can drive swimming, top-down
characters or ships. It does not link the platformer controller or tile collision.

```c
static const inertial_config_t movement = {
    .accel = 3, .drag = 2, .max_speed = 40, .drift_y = 0, .glide = 2
};
inertial_update(&ship, &movement, dir_x, dir_y); /* each direction is -1, 0, 1 */
body_integrate(&ship); /* or body_step(&ship) for tile collision */
```

All tuning uses 12.4 per tick. `accel`, `drag`, `max_speed` and `glide` must
be nonnegative. No-input horizontal velocity approaches zero; vertical velocity
approaches `drift_y` clamped to `[-max_speed, max_speed]` (negative for buoyancy,
zero for no drift). This also keeps drift at zero when `max_speed` is zero. Diagonal input
reduces each axis cap to three quarters, approximating equal overall speed.
An axis above its cap approaches the cap by `glide` before accepting input again.
Zero drag or glide preserves the corresponding momentum. Keep arithmetic sums
within `int16_t`, as with the generic body APIs.

`body_push(body, dvx, dvy, limit)` is a separate generic physics function declared
in `<zgdk/physics/body.h>`. It adds an instantaneous velocity change and clamps
both axes to `[-limit, limit]`. The limit must be nonnegative and each velocity
plus delta must fit `int16_t`. It neither moves the body nor changes collision
state. It also works with the platformer controller or free bodies. Unlike
`body_accelerate()`, it imposes a speed limit.

Run `sh tests/run-inertial.sh` for native controller/impulse checks, including
proof that they link without collision, tile flags, sprites or platformer code.

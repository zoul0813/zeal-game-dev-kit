# Room grids

Include `<zgdk/rooms.h>` and `<zgdk/rooms/transitions.h>`. Room management,
optional slide transitions and minimaps live in `zgdk.lib`; no separate world
archive is needed. Games own assets, logical tile classification, player/entity
placement, persistence and scheduling. One world is active at a time.

The active world owns ZGDK's registered logical Tilemap and the selected layer's
scroll position. Do not replace that registration or scroll the same layer from
an independent camera/stream while the world is active. Other layers may be
managed separately. Room loaders must retain this registration.

## Configuring a room grid

`rooms_init(ctx, &config, start)` copies a `rooms_config_t`. The game supplies:

| Field | Meaning |
| --- | --- |
| `size` | World width and height in rooms |
| `screen` | Actual visible screen width and height in pixels |
| `room` | Visible room rectangle in screen tiles, including HUD offsets |
| `layer` | ZVB tilemap layer to scroll |
| `tiles`, `tiles_size` | Caller-owned logical room buffer and its byte capacity |
| `load_room` | Game-owned room loader |
| `on_enter` | Optional once-per-entry game callback |
| `transition` | `NULL` for immediate replacement, or a transition strategy |
| `transition_data` | Optional strategy-specific settings |

Tiles are ZVB's native 16x16 pixels. Screen dimensions must be multiples of 16;
room rectangles must fit the screen. The library validates geometry and buffer
capacity before calling the loader. Buffers, graphics context, strategy, and
strategy data must stay alive while the world is active.

For example, Enigma's layout needs no library rebuild:

```c
#include <zgdk/rooms.h>
#include <zgdk/rooms/transitions.h>

static uint8_t room_tiles[40 * 21];
static const rooms_config_t config = {
    .size = {8, 8},
    .screen = {640, 480},
    .room = {.x = 0, .y = 4, .w = 40, .h = 21},
    .layer = 0,
    .tiles = room_tiles,
    .tiles_size = sizeof(room_tiles),
    .load_room = load_room,
    .on_enter = room_entered,
    .transition = NULL,
};
```

The demo instead supplies a 16x8 world, 320x240 screen, 20x15 room at `(0,0)`,
and `&ROOMS_TRANSITION_SLIDE`. Its fixed constants live in `src/world_layout.h`
and `src/screen.h`, outside the library.

## Loading and entering rooms

The loader signature is:

```c
zos_err_t load_room(gfx_context* ctx, uint8_t room_x, uint8_t room_y,
                    uint8_t tile_x, uint8_t tile_y);
```

`tile_x` and `tile_y` are **absolute tilemap coordinates**, already including
viewport offsets and transition staging. They are not screen indices. The game
reads/decompresses its map, uploads graphics at that origin, and populates the
registered logical map using room-local `tilemap_set_xy(col, row, type)` calls.
The loader must use the supplied room identity, not `rooms_position`, because
transitions may stage several rooms while the source remains current.

Call `rooms_move(edge)` to enter a neighbor. It returns `1` on success, `0` at a
world boundary, or a negated error code. `rooms_neighbor(edge, &destination)`
queries a neighbor without loading it. Games own movement, boundary clamping,
and arrival positions.

`on_enter(ctx, room_x, room_y, edge)` runs exactly once after all loading and
presentation succeed and `rooms_position` is committed. Initialization also
calls it, with `EdgeNone`. This is where an Enigma adapter can apply its room
palette, place its objects, and spawn its enemies. Those functions remain in
the game. Callbacks must not recursively change/reinitialize the world.

A failed load or presentation does not commit the destination or invoke
`on_enter`. Because the game loader may already have modified logical/visible
map data, the active world is invalidated; `rooms_get_config()` returns `NULL`
and further moves fail. The game must stop or successfully reinitialize before
continuing. Invalid configuration is rejected before replacing an active world.

## Optional transitions

With `transition = NULL`, the core loads once at the visible room rectangle,
keeps the viewport at tilemap origin `(0,0)`, and performs no animation.

The demo's slide is selected by including `zgdk/rooms/slide.h` and assigning
`&ROOMS_TRANSITION_SLIDE`. It requires a full-screen room and space for three
horizontal and two vertical staging screens in ZVB's 80x40 tilemap. Unsupported
layouts are rejected during initialization; Enigma's 640x480 layout uses the
immediate path.

`rooms_slide_config_t` optionally provides `draw_frame(ctx, edge, progress)`
through `transition_data`. It runs during vblank after each scroll step, with
progress in pixels. The game can animate/render its own visuals there. The
current demo keeps its short walk off screen and arrival positioning in game
code, preserving the existing slide without teaching the library about sprites.

A new transition can implement `rooms_transition_t`: optional `prepare()`
validates geometry and chooses the stable viewport origin without drawing;
`run()` stages/animates using the source, destination, edge, and origin in its
request. The core then reloads the destination at the stable origin, presents
it, commits identity, and notifies the game once.


## Independent minimaps and validation

[Minimaps](Minimap.md) do not depend on world management. The game connects them
by supplying position updates. `sh tests/run-rooms.sh` runs sanitizer tests with
actual SDK tilemap functions. After building, run
`python3 tests/check-rooms-target.py` inside ZDE for both SDCC room layouts,
initializer byte checks and optional archive extraction. Immediate rooms do not
link slide code, and state-only minimaps link neither world nor graphics.

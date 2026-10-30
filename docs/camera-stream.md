# Cameras and tile streaming

`zgdk/camera.h` and `zgdk/tilemap/stream.h` work independently of collision,
physics, entities, files, and the global `Tilemap`. They allocate no memory.
The Video Board uses 16-pixel tiles and an 80 by 40 ring per layer.

## Camera

```c
camera_t camera;
camera_init_view(&camera, 1600, 960, 320, 240); /* check ERR_SUCCESS */
camera.lead = 48;
camera.max_step = 6; /* zero means instant */
camera_follow_target(&camera, player_x, player_y, 12, 30, direction);
/* Or camera_set(&camera, x, y) for a manual camera. */
camera_scroll_layer(&camera, 1, 0, 0); /* during vblank */
camera_scroll_layer(&camera, 0, 1, 2); /* half x speed, quarter y speed */
```

Targets use signed top-left pixels and unsigned dimensions, rather than ZGDK's
bottom-right `Rect` convention. Direction is negative, zero, or positive.
The default deadzone leaves one third of the view as each horizontal margin
and three eighths as each vertical margin. Margins are editable pixel values.
Following applies directional lead, a per-axis step limit, then world clamping.
Small worlds clamp to zero. World-minus-view bounds must fit signed 16 bits;
zero dimensions are invalid. All functions returning `zos_err_t` must be checked.
Each public camera function lives in its own compilation unit under
`src/camera/`, so unused functions stay out of the linked program. Static axis
helpers remain with the function that uses them. Following pulls in
`camera_set()` as its clamping dependency. Reader
initialization, tile editing, buffer operations, and camera rebasing also use
separate archive members so unused capabilities cost no program memory.

Scroll shifts range from 0 to 15 independently per axis. `origin_x/y` retain
the ordinary hardware remainder. The unsigned 32-bit `phase_x/y` accumulators
count whole hardware periods modulo 32768. Together these represent phases
modulo 1280*32768 and 640*32768 pixels, preserving every supported shift even
after indefinitely repeated rebases. Camera functions maintain all phase fields.

## RAM source

```c
uint8_t tiles[100 * 60]; /* caller fills this; keep alive while streaming */
tilemap_stream_t stream;
zos_err_t err = tilemap_stream_init_ram(ctx, &stream, tiles, 100, 60, 1,
                                       320, 240, camera.x, camera.y);
/* Each frame, after moving the camera: */
err = tilemap_stream_update(ctx, &stream, camera.x, camera.y);
/* Single edit, including a resident VRAM update: */
err = tilemap_stream_set_tile(ctx, &stream, tile_x, tile_y, new_tile);
/* After bulk edits: */
err = tilemap_stream_redraw(ctx, &stream);
```

RAM sources require a complete addressable buffer, at most 65535 bytes. The
caller must also ensure the actual buffer fits in the target address space;
no banking is provided. Source dimensions range from 1 to 32767 tiles. Viewport
coverage is `ceil((pixels + 15) / 16)` on each axis: a 320 by 240 view retains
21 by 16 tiles, including every tile that sub-tile movement might expose.
The maximum coverage is 80 by 40. Source reads are clipped; cells outside the
source map keep their existing hardware contents, so applications needing a
blank border should fill the layer first.

## Procedural or file-backed reader

```c
static zos_err_t terrain(void *source, uint16_t x, uint16_t y,
                         uint8_t *dst, uint8_t count) {
    uint8_t i;
    (void)source;
    for (i = 0; i < count; ++i) dst[i] = (y > 40 && ((x + i) % 7)) ? 1 : 0;
    return ERR_SUCCESS;
}
uint8_t row_scratch[21];
tilemap_stream_t terrain_stream;
zos_err_t err = tilemap_stream_init_reader(ctx, &terrain_stream,
    terrain, NULL, row_scratch, sizeof row_scratch, 2000, 2000, 1,
    320, 240, camera.x, camera.y);
```

The callback is synchronous. Success supplies exactly `count` consecutive
byte-sized tile indices from the requested row. It may be called for a full
viewport row or for a single tile in a column. Scratch storage must hold at
least one coverage row and stay alive while streaming. Calls sharing scratch
must not overlap or re-enter streaming.

A file callback owns its handle and lifetime, row offset calculations, exact
reads, page-safe destination handling, caching, decompression, and all errors.
ZGDK neither opens nor closes files. Callback errors propagate unchanged;
graphics errors become `ERR_FAILURE`. A partial failure marks the stream dirty,
and the next update redraws the entire window even if the position is unchanged.
Reader sources cannot use `set_tile` or `shift_buffer` (`ERR_NOT_SUPPORTED`):
update the source yourself, then redraw or rebase as appropriate.

## Multiple layers and rebasing

Use one stream per layer, with independent map data and viewport positions.
For a parallax tile source, pass that layer's divided viewport coordinates to
its stream and matching shifts to `camera_scroll_layer`. A repeated background
that already fills the hardware ring only needs camera scrolling.

For an endless RAM world, compact before reaching the buffer edge. Reserve at
least view coverage plus the dropped rows/columns; refill vacated cells before
they become resident. Coordinate the independent operations once per rebase:

```c
/* Validate offsets against all participants before starting this sequence. */
tilemap_stream_shift_buffer(&stream, drop_columns, drop_rows);
tilemap_stream_rebase(&stream, drop_columns, drop_rows);
camera_rebase(&camera, drop_columns * 16, drop_rows * 16);
/* Shift application entities by the same negative pixel offsets. */
/* Refill the vacated source cells, then resume stream_update. */
```

Compaction copies the retained rectangle toward zero, preserving vacated bytes;
it does not change ring state. Rebasing changes only loaded coordinates and
ring offsets, without drawing or editing data. Camera rebasing changes local
position and phase but leaves world bounds intact. Only nonnegative offsets
are accepted, and resulting local positions must fit signed 16 bits.
With multiple streamed layers, compact/rebase each relevant source but move the
camera and entities once. A reader's owner must implement the corresponding
source-coordinate change itself. Parallax layers may need different tile offsets
and retain fractional phase; do not blindly apply a playfield tile rebase to them.

Run `tests/run-stream.sh` for host tests with address and undefined-behavior
sanitizers. It needs only a C compiler and the included platform stubs.

The optional `tests/stream/target` program builds with the real SDCC/Zeal
toolchain (`cmake -B build && cmake --build build` in that directory). It uses
software graphics stubs and checks negative clipping under 16-bit integer
promotion, coordinate limits, error recovery, and 43,000 rebases across all
parallax shifts. Run `build/sdktest.bin` in the emulator and break at `test_done`;
`test_result` must be 1 and `test_failure` must be zero. Symbol addresses are
in the generated map.

The same native test build also links `caminit`, an initialization-only program.
Its post-link map check requires `camera_init_view` and rejects every other
public camera function and hardware scrolling, proving archive extraction
excludes unused camera modules.

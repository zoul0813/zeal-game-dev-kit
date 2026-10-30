# Cameras and tile streaming

`zgdk/camera.h` and `zgdk/tilemap/stream.h` work independently of collision,
physics, entities, files, and the global `Tilemap`. They allocate no dynamic
memory; callers own camera/stream state, source data, and reader scratch storage.
The Video Board tilemap uses 16-pixel tiles and an 80 by 40 ring per layer.
API call snippets run inside a function returning `zos_err_t`, with an
initialized `gfx_context *ctx` where required. Define reader callbacks at file
scope. Include the corresponding headers and `<stddef.h>` for `NULL`.

## Camera

```c
camera_t camera;
zos_err_t err = camera_init_view(&camera, 1600, 960, 320, 240);
if (err != ERR_SUCCESS) return err;
camera.lead = 48;
camera.max_step = 6; /* zero means instant */
camera_follow_target(&camera, player_x, player_y, 12, 30, direction);
/* Or camera_set(&camera, x, y) for a manual camera. */
err = camera_scroll_layer(&camera, 1, 0, 0); /* during vblank */
if (err != ERR_SUCCESS) return err;
err = camera_scroll_layer(&camera, 0, 1, 2); /* half x speed, quarter y speed */
if (err != ERR_SUCCESS) return err;
```

Targets use signed top-left pixels and unsigned dimensions, rather than ZGDK's
bottom-right `Rect` convention. Direction is negative, zero, or positive.
The default deadzone leaves one third of the view as each horizontal margin
and three eighths as each vertical margin, rounded down to whole pixels.
Margins, `lead`, and `max_step` are editable; other fields are library-managed.
Following applies directional lead, a per-axis step limit, then world clamping.
Small worlds clamp to zero. All functions returning `zos_err_t` must be checked.

### Limits

All camera math is 16-bit, with no 32-bit runtime and no division per frame.
These ranges keep following intermediates inside `int16_t` and unsigned
scroll/rebase intermediates inside `uint16_t`, so results are exact.
Initialization, scroll arguments, and rebase offsets are validated as listed
below. Target and deadzone values outside their ranges are not detected and can
give a wrong camera position.

| Value | Range | Checked by |
|---|---|---|
| World size, view size | 1 to `CAMERA_MAX_SIZE` (16,383) px per axis | `camera_init_view` |
| Camera `x`, `y` | 0 to `CAMERA_MAX_SIZE - 1` | `camera_set` clamps (don't write larger values into the fields) |
| Follow target `x`, `y` (top-left) | `CAMERA_TARGET_MIN` (-8,192) to `CAMERA_MAX_SIZE - 1` | not checked |
| Follow target `w`, `h` | 0 to `CAMERA_MAX_TARGET` (8,191) px | not checked |
| Deadzone margins | 0 to the view size on that axis | not checked |
| `lead`, `max_step` | 0 to 255 (`uint8_t`) | by type |
| Parallax shift | 0 to `CAMERA_MAX_SHIFT` (4, 1/16 speed) | `camera_scroll_layer` |
| `camera_rebase` offsets | 0 to the current `x` / `y` (the camera never goes negative) | `camera_rebase` |

16,383 px is 1,023 whole tiles of 16 px per axis; a RAM map 1,023 by 64 tiles
uses 65,472 bytes, just under 64 KiB. Longer worlds stream and rebase, so the
camera only ever sees the local window. Signed 12.4 fixed-point positions have
a maximum whole-pixel value of 2,047. Targets may sit partly or fully outside
the world (a respawn off screen, say): the camera clamps to the
world edge.

Each public camera function lives in its own compilation unit under
`src/camera/`, so unused functions stay out of the linked program. Static axis
helpers remain with the function that uses them. Following pulls in
`camera_set()` as its clamping dependency. Reader initialization, RAM
initialization, tile editing, RAM compaction, stream rebasing, and camera rebasing also use separate archive members so unused capabilities
cost no program memory. Ring indexing uses bounded subtraction instead of the
software division runtime; a redraw computes its initial ring position once
and advances the row with a compare and wrap.

Scroll shifts range from 0 to `CAMERA_MAX_SHIFT` (4, 1/16 speed) independently
per axis. `origin_x/y` hold the pixels removed by `camera_rebase`, modulo
1280 << 4 and 640 << 4. That keeps every supported shift exact after any number
of rebases, and keeps all scrolling arithmetic in 16 bits with no division:
`camera_scroll_layer` uses addition, shifting, and bounded subtraction per axis.
Its execution time within a game's vblank budget has not been profiled.
Camera positions stay 0 to `CAMERA_MAX_SIZE - 1` (see Limits).

## RAM source

```c
static uint8_t tiles[100 * 60]; /* caller fills this; keep alive while streaming */
tilemap_stream_t stream;
zos_err_t err = tilemap_stream_init_ram(ctx, &stream, tiles, 100, 60, 1,
                                       320, 240, camera.x, camera.y);
if (err != ERR_SUCCESS) return err;
/* Each frame, after moving the camera: */
err = tilemap_stream_update(ctx, &stream, camera.x, camera.y);
if (err != ERR_SUCCESS) return err;
/* Single edit, including a resident VRAM update: */
err = tilemap_stream_set_tile(ctx, &stream, tile_x, tile_y, new_tile);
if (err != ERR_SUCCESS) return err;
/* After bulk edits: */
err = tilemap_stream_redraw(ctx, &stream);
if (err != ERR_SUCCESS) return err;
```

RAM sources require a complete addressable buffer, at most 65535 bytes. The
caller must also ensure the actual buffer fits in the target address space;
no banking is provided. RAM data and reader scratch passed to the SDK row loader
must remain readable while it maps VRAM into CPU page 0 (addresses
`0x0000..0x3FFF`); do not place that storage in page 0.
Source dimensions range from 1 to 32767 tiles per axis. View dimensions range
from 1 to 1265 pixels wide and 1 to 625 pixels high. Stream initialization and
updates accept signed 16-bit top-left pixels (`-32768..32767`), independently
of the narrower camera limits. Viewport coverage is `ceil((pixels + 15) / 16)` on each axis: a 320 by 240 view retains
21 by 16 tiles, including every tile that sub-tile movement might expose.
The maximum coverage is 80 by 40. Source reads are clipped; cells outside the
source map keep their existing hardware contents, so applications needing a
blank border should fill the layer first. Invalid initialization parameters leave
the stream unchanged; a drawing failure leaves the initialized stream dirty.
`set_tile` changes RAM before attempting the VRAM write; that RAM edit remains
even if rendering fails.

## VRAM transfers and update behavior

Each streamed tile is one byte: a tile index referencing graphics already in
VRAM. A full 320 by 240 viewport refresh transfers up to 336 index bytes, not
336 tiles of pixel graphics.

Rows use the Video Board SDK's `gfx_tilemap_load()`, which maps VRAM and copies
with CPU `memcpy()`. Rows crossing the ring edge use two copies. Columns use
`gfx_tilemap_place()` once per tile, mapping and restoring VRAM for each call.
This streaming path does not use DMA.

An update within the same tile performs no transfers unless the stream is dirty.
Moving one tile per axis loads the exposed column and/or row. Moving more than
one tile on either axis redraws the entire coverage window; dirty streams also
redraw. The relative performance of incremental and full redraws, and of CPU and DMA
transfers, has not been established for representative game workloads.

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
if (err != ERR_SUCCESS) return err;
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
err = tilemap_stream_shift_buffer(&stream, drop_columns, drop_rows);
if (err != ERR_SUCCESS) return err;
err = tilemap_stream_rebase(&stream, drop_columns, drop_rows);
if (err != ERR_SUCCESS) return err;
err = camera_rebase(&camera, drop_columns * 16, drop_rows * 16);
if (err != ERR_SUCCESS) return err;
/* Shift application entities by the same negative pixel offsets. */
/* Refill the vacated source cells, then resume stream_update. */
```

Compaction copies the retained rectangle toward zero, preserving vacated bytes;
it does not change ring state. Rebasing changes only loaded coordinates and
ring offsets, without drawing or editing data. Camera rebasing changes local
position and origin but leaves world bounds intact. Camera offsets may not
exceed the current local pixel position, so the camera never goes negative.
Stream rebase offsets must be smaller than the source dimensions and must keep
loaded tile coordinates within `int16_t`; unlike the camera, those coordinates
may become negative. All offsets are unsigned, and zero is allowed. Each
operation validates its own inputs, but the sequence is not a transaction.
With multiple streamed layers, compact/rebase each relevant source but move the
camera and entities once. A reader's owner must implement the corresponding
source-coordinate change itself. Parallax layers may need different tile offsets
and keep fractional parallax position; do not blindly apply a playfield tile rebase to them.

## Validation

Run `tests/run-stream.sh` for host tests with address and undefined-behavior
sanitizers. It needs only a C compiler and the included platform stubs.

The optional `tests/stream/target` program builds with the real SDCC/Zeal
toolchain (`cmake -S tests/stream/target -B build/stream-target` followed by
`cmake --build build/stream-target` from the repository root). Set `ZOS_PATH`,
`ZVB_SDK_PATH`, `COREUTILS_PATH`, and `ZGDK_PATH` to their checkouts and build the
root library first (`cmake -S . -B build/library`, then
`cmake --build build/library`), since the tests link `lib/zgdk.lib`. It uses
software graphics stubs and checks negative clipping under 16-bit integer
promotion, coordinate limits, error recovery, and 43,000 rebases across all
parallax shifts (0 to `CAMERA_MAX_SHIFT`). Run
`build/stream-target/sdktest.bin` in the emulator and break at `test_done`;
`test_result` must be 1 and `test_failure` must be zero. Symbol addresses are
in the generated map.

The same native test build also links `caminit`, an initialization-only program.
Its post-link map check requires `camera_init_view` and rejects every other
public camera function and hardware scrolling, proving archive extraction
excludes unused camera modules.

The native build also links `readeronly`, a procedural stream using rebasing.
Its map check rejects RAM initialization, RAM compaction, `mem_cpy`, and integer
division/modulus runtimes. Host tests exhaustively check ring indexing over all
16-bit coordinates and every valid ring offset.

A successful native build validates compilation and link dependencies. Runtime
checks require the emulator or hardware; these tests do not measure performance.

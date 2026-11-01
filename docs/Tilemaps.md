# ZGDK: Tilemaps

ZGDK provides some convenient functions for managing tilemaps.

## Load a tile buffer

```c
#include <zgdk/tilemap.h>

static uint8_t tiles[20 * 15];  /* Fill with tile IDs before loading. */
static Tilemap map;

zos_err_t err = tilemap_load(&ctx, &map, tiles, 20, 15, LAYER0, 0, 0);
```

`tilemap_load(ctx, tilemap, tiles, width, height, layer, x, y)` uploads a tightly
packed row-major byte buffer to the chosen layer. Dimensions and destination
coordinates are in tiles. Layers are 0 or 1; the nonempty rectangle must fit
within the 80x40 hardware tilemap without wrapping. Use streaming for larger
maps or camera-driven ring updates.

After all rows upload successfully, the function fills `tilemap->rect` with
`{x,y,width,height}`, stores the supplied buffer pointer in `tilemap->tiles`,
and registers the map as `_TILEMAP`. It returns `ERR_SUCCESS`. There is no heap
allocation or tile-buffer copy: the caller retains ownership and must keep the
buffer and descriptor valid while registered. Edits to the buffer do not redraw
the screen automatically. `tilemap_get_xy()` reads local buffer coordinates;
`tilemap_get(Point *)` subtracts the registered rectangle's origin.
For individual redraws at a nonzero destination, use `tilemap_place_xy()` or
`gfx_tilemap_place()` with hardware tile coordinates. The existing
`tilemap_place(Point *)` helper still subtracts the map origin before drawing.

Null pointers, zero dimensions, invalid layers and out-of-bounds rectangles
return `ERR_INVALID_PARAMETER` before any drawing. Graphics upload errors are
reported as `ERR_FAILURE`, matching the streaming APIs' OS error namespace.
The descriptor and global registration are unchanged on failure, including when
reloading an already registered descriptor. Earlier rows uploaded before a
failure remain visible; the operation does not roll back VRAM writes.

The ZVB upload routine temporarily maps VRAM into CPU page 0. On target, keep
the input buffer outside that page, and use an initialized
graphics context. Collision/property caches belong to their consumers: refresh
them after a successful load if needed. No tile-property meanings or entity
spawning rules are part of this API.

Run `sh tests/run-tilemap.sh` for loader bounds, offset rectangles, registration
and injected upload-failure tests under ASan/UBSan.

## Register and access maps

You can register a tilemap with `tilemap_register(&tilemap, tiles)`

You can also get and set tiles using `tilemap_get(&point)` and `tilemap_set(&point, index)`

You can also use `tilemap_get_xy` and `tilemap_set_xy` to pass `uint8_t x, y;` values instead.

Use `tilemap_load` to draw a complete buffer. To place an individual tile, call `tilemap_place(&vctx, LAYER0, index, &point)`

Alternatively, you can also call `tilemap_place_xy` to pass `uint8_t x, y;` values instead.

`tilemap_fill` and `tilemap_fill_rect` are available for filling an area with identical tiles (useful for clearing a layer, or 
populating a background, etc).

`tile_get_at_point` accepts tilemap pixel coordinates; `tile_at` accepts tile indices. Both return tile indices in `Tile.x` and `Tile.y`, and a rectangle in sprite coordinates in `Tile.rect`. The rectangle includes layer-0 scroll only. If your collision map uses independently scrolled layer 1, adjust the returned rectangle for that layer before using it.

For independent cameras, larger maps, procedural readers, and rebasing, see
[Cameras and tile streaming](camera-stream.md).

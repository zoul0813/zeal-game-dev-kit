# Independent minimaps

Include `<zgdk/minimap/state.h>` for state only, `<zgdk/minimap/render.h>` for
rendering, or `<zgdk/minimap.h>` for the combined convenience API. No rooms
module is required: games provide grid dimensions and room-position changes.
The state is a single active minimap with caller-owned storage.


`zgdk/minimap/state.h` takes a world size and two caller-owned bitsets, each at least
`MINIMAP_STATE_BYTES(width, height)` bytes and non-overlapping. It tracks
unvisited, visited, and game-defined special rooms independently of the rooms
module. Updating position records the previous room's current special flag.

The optional two-tile renderer supports up to 16x8 rooms and 4-bit or 8-bit
graphics. `minimap_render_config_t` supplies placement, layer, two reserved tile
IDs, and palette indices for unvisited/visited/special/current. Four-bit colors
must be in `0..15`. The current-room color overlays the three room states.
Larger maps can reuse the state API with their own renderer.

The convenience API is `minimap_init(ctx, &state_config, &render_config, start)`
followed by `minimap_set_position(ctx, position, previous_is_special)`. The demo
uses its unopened-chest flag for "special"; the library has no chest knowledge.

Invalid initialization parameters, including unsupported renderer dimensions,
colors or tile IDs, leave the existing configuration and exploration bitsets
unchanged. A graphics failure during otherwise valid initialization disables the
convenience API until initialization succeeds again.

If `minimap_set_position` fails, it restores the prior position and visited/special
flags. Retry the same call with the same `previous_is_special` value after fixing
the graphics error. Partial graphics writes are not rolled back; the successful
retry redraws both tiles from the restored state and requested move.


The state initializer rejects overlapping active visited/special bitset spans
before clearing either buffer or replacing the existing configuration. Adjacent
spans are allowed; unused capacity does not participate in the overlap check.
Keep both buffers alive while the minimap is active.

The renderer uploads two tiles and places their IDs on the selected tilemap
layer; it does not allocate sprites or register a logical Tilemap. Rendering
uses a 512-byte shared scratch buffer. State-only users do not link that buffer,
graphics, slide transitions or world state. Repeated updates replace tile pixels;
placement remains at the configured location.

`sh tests/run-rooms.sh` covers storage, overlap rejection, live room flags,
4/8-bit packing, dimensions and graphics failures. After building in ZDE,
`python3 tests/check-rooms-target.py` checks target ABI and independent linking.

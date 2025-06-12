# ZGDK: Tilemaps

ZGDK provides some convenient functions for managing tilemaps.

You can register a tilemap with `tilemap_register(&tilemap, &tiles)`

You can also get and set tiles using `tilemap_get(&point)` and `tilemap_set(&point, index)`

You can also use `tilemap_get_xy` and `tilemap_set_xy` to pass `uint8_t x, y;` values instead.

Tilemaps are not directly rendered to the display, to render the tilemap you can call `tilemap_place(&vctx, LAYER0, index, &point)`

Alternatively, you can also call `tilemap_place_xy` to pass `uint8_t x, y;` values instead.

`tilemap_fill` and `tilemap_fill_rect` are available for filling an area with identical tiles (useful for clearing a layer, or 
populating a background, etc).

# Tilemap HUD text

Include `<zgdk/hud.h>` or `<zgdk/hud/tilemap.h>`; `<zgdk.h>` includes the HUD
umbrella. These functions write ASCII-mapped font tiles to a graphics tilemap,
commonly used for overlay text on `LAYER1`.

```c
void hud_tilemap_put_s(gfx_context *ctx, const char *str, uint8_t x, uint8_t y);
void hud_tilemap_put_s_layer(gfx_context *ctx, const char *str,
                          uint8_t layer, uint8_t x, uint8_t y);
void hud_tilemap_nput_s(gfx_context *ctx, const char *str,
                      uint8_t len, uint8_t x, uint8_t y);
void hud_tilemap_nput_s_layer(gfx_context *ctx, const char *str, uint8_t len,
                           uint8_t layer, uint8_t x, uint8_t y);
```

Coordinates are in tiles. Forms without a layer argument use `LAYER1`; the
explicit-layer forms preserve support for either tile layer. `hud_tilemap_put_s*`
reads a NUL-terminated string. `hud_tilemap_nput_s*` reads exactly `len` bytes,
which need not end in NUL.

Load the font tiles first, then use `<zgdk/ascii.h>` to map printable ASCII
characters to tile IDs. For example, after loading A–Z at tile 160:

```c
#include <zgdk/ascii.h>
#include <zgdk/hud.h>

ascii_map('A', 26, 160);
hud_tilemap_put_s(&ctx, "READY", 2, 3);
```

The existing rendering behavior is preserved. Mapping ranges and text characters
must remain in printable ASCII 32–126. Each call must be **at most 40 characters**
and fit the destination tilemap; the 40-byte temporary buffer performs no length
validation. There is no newline handling or wrapping. These void functions do not
report tilemap-write errors.

## Compatibility

`<zgdk/utils/print.h>` and `<zgdk/hud/print.h>` remain forwarding headers. Old includes through
`<zgdk/utils.h>` and `<zgdk.h>` also continue to work.

| Existing name | Canonical name |
|---|---|
| `print_string` | `hud_tilemap_put_s` |
| `print_string_layer` | `hud_tilemap_put_s_layer` |
| `nprint_string` | `hud_tilemap_nput_s` |
| `nprint_string_layer` | `hud_tilemap_nput_s_layer` |
| `hud_print_string` | `hud_tilemap_put_s` |
| `hud_print_string_layer` | `hud_tilemap_put_s_layer` |
| `hud_nprint_string` | `hud_tilemap_nput_s` |
| `hud_nprint_string_layer` | `hud_tilemap_nput_s_layer` |

The old names are automatically available through `hud/deprecated.h`. They emit
a deprecation warning when used, including when taking their address; merely
including a header is quiet. Source aliases call the canonical functions directly
and add no wrapper code. Compatibility covers source recompiled with these
headers; there are no separate legacy linker symbols.

Each canonical function occupies its own archive member.
SDK implementation code uses the canonical HUD names.

Run `sh tests/run-hud.sh` for native sanitizer, compatibility and diagnostic
checks, and `python3 tests/check-hud-compat.py --sdcc` with the SDK/toolchain
environment set to verify SDCC warnings and alias code size. After rebuilding
`zgdk.lib`, `python3 tests/check-hud-link.py` checks exports and member isolation.

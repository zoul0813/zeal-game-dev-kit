# HUD text

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

## Sprite HUD text

Include `<zgdk/hud/sprite.h>` or `<zgdk/hud.h>`. These helpers modify an existing
caller-owned contiguous sprite range. They neither allocate/register sprites nor
write VRAM. Games retain their sprite arena, dirty tracking and upload schedule.
The descriptor is a four-byte view on Z80 and can be a local variable; the SDK owns
no persistent HUD state.

```c
hud_sprite_text_t line = {banner_sprites, 20, 12};
hud_sprite_put_s(&line, "READY", 20, 40);
hud_sprite_put_s_centered(&line, "READY", 160, 40);
hud_sprite_nput_s(&line, buffer, length, 20, 40); /* no NUL required */
hud_sprite_hide(banner_sprites, 20);
/* Upload the changed range during vblank, using the game's existing renderer. */
```

The fields are `sprites`, `capacity` and `spacing`. Capacity is the number
of writable sprites and spacing is the horizontal pixel advance. Coordinates are
screen pixels; the helpers add the hardware's +16 sprite origin bias. Centering
uses the truncated string's spacing cells, including spaces. Keep the resulting
left position within `int16_t`. There is no wrapping or viewport clipping.

`hud_sprite_put_s()` and the centered form read at most `capacity` bytes, stopping
at NUL. `hud_sprite_nput_s()` reads exactly `min(length, capacity)` bytes. Spaces,
non-printable characters and unused slots are hidden by setting hardware Y to
512 (screen Y=496), below every supported display mode. This also hides tall
sprites without changing their options and permits early hardware Y rejection.
A NULL string clears the range. A NULL view/range or zero capacity is a no-op.
The helpers preserve sprite flags/options, so palette, flipping and tile-bank
selection remain caller-controlled.

Sprite and tilemap text both use `ASCII_MAP` through `ascii_to()`. Games define
font locations with `ascii_map()` before drawing text; changing the shared map
changes subsequent text in both renderers. There is no per-view map or raw-character
mode. Space sprites stay hidden as layout gaps. Font tiles remain game assets.
Games can map unused printable characters to icon tiles and name them with string
macros. The names describe the glyph, while the character encoding stays in one
place. For example (tile constants belong to the game):

```c
#define LIFE_HEART_FULL "%"
#define LIFE_HEART_HALF "/"
ascii_map(LIFE_HEART_FULL[0], 1, TILE_HEART_FULL);
ascii_map(LIFE_HEART_HALF[0], 1, TILE_HEART_HALF);
hud_sprite_put_s(&line, "Lives: " LIFE_HEART_FULL LIFE_HEART_FULL LIFE_HEART_HALF, 20, 40);
```

The same string works with tilemap text. In a dynamically assembled buffer, use
`LIFE_HEART_FULL[0]` to append its character. An icon occupies one spacing cell;
choose spacing to fit the artwork. A mapped character replaces its usual glyph
for both renderers, so reserve characters the game does not need as punctuation.
Space remains a hidden sprite gap. Mapping does not recolor the icon or change
sprite flags/options.

Each public capability is in its own archive member; hiding a range alone links
no font mapping or text rendering code.

Run `sh tests/run-hud-sprite.sh` for native sanitizer checks, including non-NUL
buffers, truncation, stale-character clearing, glyph metadata and adjacent-range
preservation. The sprite implementation links without arena or VRAM code; drawing text shares
the SDK ASCII table.

After rebuilding the library, run `python3 tests/check-hud-sprite-link.py` with
SDCC and `ZOS_PATH` / `ZVB_SDK_PATH` set. It checks the four-byte Z80 view, mixed
tilemap/sprite use and selective linking with a shared font table and no arena dependency.

Run `sh tests/run-hud-sprite-z80.sh` with SDCC and Python `z80` installed to
execute stack-local sprite text descriptors on the Z80. This checks long menu
captions, capacity limits and centering against the linked archive, catching
compiler pointer-drift regressions that native C tests cannot expose.

## Reusable SNES pad artwork

[`assets/snes-pad.gif`](../assets/snes-pad.gif) provides thirteen 16 × 16
controller glyphs with a magenta palette index 0 marked transparent. Copy and recolor
these optional source tiles into the game's tileset, choosing transparency or
an opaque background to match its font. See the [artwork guide](../assets/README.md)
for tile order and import details. The artwork adds no code or data to `zgdk.lib`.

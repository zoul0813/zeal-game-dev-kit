# Utilities

ZGDK utilities cover tile-based text, random numbers, formatted strings, logging,
emulator timing markers, directory creation and byte-grid row addressing.
Prefer the individual header when only one facility is needed.

`<zgdk/utils.h>` includes the following headers; `<zgdk.h>` includes that umbrella
in turn. String aliases and directory creation require explicit includes.

| Header | Purpose | Included by utils.h |
|---|---|---|
| [`utils/print.h`](../include/zgdk/utils/print.h) | Render ASCII strings as tiles | Yes |
| [`utils/rand.h`](../include/zgdk/utils/rand.h) | Z80 refresh-register/LFSR random bytes | Yes |
| [`utils/string.h`](../include/zgdk/utils/string.h) | Integer conversion and simplified formatted output | No |
| [`utils/_strlen.h`](../include/zgdk/utils/_strlen.h) | SDK string-length helper and strlen alias | No |
| [`utils/log.h`](../include/zgdk/utils/log.h) | Console error and string messages | Yes |
| [`utils/debug.h`](../include/zgdk/utils/debug.h) | Emulator T-state markers | Yes |
| [`utils/ensure_parent_dirs.h`](../include/zgdk/utils/ensure_parent_dirs.h) | Create a path's parent directories | No |
| [`utils/grid_rows.h`](../include/zgdk/utils/grid_rows.h) | Initialize byte-grid row pointers | Yes |

## Inline helpers

Include `<zgdk/inline.h>`, or use `<zgdk.h>` which includes it. Header-defined
helpers can use `INLINE` directly:

```c
#include <zgdk/inline.h>

INLINE void foo(void) {
    /* Small helper body. */
}
```

`INLINE` expands to plain `inline` on SDCC and `static inline` on host compilers.
SDCC can emit unused out-of-line bodies for `static inline` functions; using
plain inline avoids those extra copies. Host static linkage provides a callable
fallback for unoptimized tests and permits the same header in multiple C files.

On SDCC, the definition must be visible where called and the compiler must be
able to inline it: this macro supplies no out-of-line function definition. If
a helper needs a function pointer or cannot inline, put a regular definition in
one C file and a declaration in its header instead. No rebuilt `zgdk.lib` is
needed for this header-only macro.

Run `sh tests/run-inline.sh` to check shared-header linkage across two
unoptimized host translation units under ASan/UBSan.

## Tile-based text

Include `<zgdk/utils/print.h>`. These functions render to a graphics tilemap;
they do not write to the console.

```c
void print_string(gfx_context *ctx, const char *str, uint8_t x, uint8_t y);
void print_string_layer(gfx_context *ctx, const char *str,
                        uint8_t layer, uint8_t x, uint8_t y);
void nprint_string(gfx_context *ctx, const char *str,
                   uint8_t len, uint8_t x, uint8_t y);
void nprint_string_layer(gfx_context *ctx, const char *str, uint8_t len,
                         uint8_t layer, uint8_t x, uint8_t y);
```

Coordinates are in tiles. The forms without a layer argument use `LAYER1`.
`print_string*` reads a NUL-terminated string; `nprint_string*` reads exactly
`len` bytes, which need not end in NUL.

Each byte is translated through the ASCII-to-tile mapping from
`<zgdk/ascii.h>`. Load the font tiles first, then use `ascii_map(start, length,
from)` to associate characters with consecutive tile IDs. For example, after
loading A–Z at tile 160:

```c
#include <zgdk/ascii.h>
#include <zgdk/utils/print.h>

ascii_map('A', 26, 160);
print_string(&ctx, "READY", 2, 3);
```

The mapping covers printable ASCII 32–126; `ascii_to(c)` returns the mapped
tile ID. Mapping ranges and text characters must remain within that range.
The current print implementation uses a 40-byte temporary array and performs
no length validation: keep each call to **at most 40 characters**, and ensure
the text fits the destination tilemap. It does not interpret newlines or wrap
text. The functions return void and do not expose tilemap-write errors.

## Random bytes

Include `<stdint.h>` before `<zgdk/utils/rand.h>` when including that header
directly; its seed declaration uses `uint16_t`.

```c
void rand8_seed(uint16_t seed);
char rand8(void);
char rand8_quick(void);
```

`rand8()` advances a 16-bit LFSR and mixes in the Z80 refresh register `R`.
Its initial seed is 1; `rand8_seed(nonzero_seed)` replaces it, while seed 0
leaves it unchanged. `rand8_quick()` simply returns `R` and does not use the
LFSR seed. Cast either result to `uint8_t` when treating it as a value in 0–255:

```c
#include <stdint.h>
#include <zgdk/utils/rand.h>

rand8_seed(0x5eed);
uint8_t value = (uint8_t)rand8();
```

A fixed seed does **not** guarantee a reproducible sequence: `R` depends on
executed instructions, so code changes and timing can change the results.
Replay tests that need an identical sequence must supply a deterministic RNG.

## Integer conversion and formatted output

Include `<zgdk/utils/string.h>` explicitly. It defines these aliases:

| Alias | ZGDK function |
|---|---|
| `itoa` | `zgdk_itoa` |
| `sprintf` | `zgdk_sprintf` |
| `printf` | `zgdk_printf` |
| `fprintf` | `zgdk_fprintf` |

These macro aliases are why the header is not included by `<zgdk/utils.h>`.
They select ZGDK's implementations, whose signatures and formatting support
differ from the standard C functions.

```c
void zgdk_itoa(int num, char *str, uint8_t base, char alpha);
int zgdk_sprintf(char *buffer, const char *format, ...);
void zgdk_printf(const char *format, ...);
void zgdk_fprintf(zos_dev_t dev, const char *format, ...);
```

`zgdk_itoa` writes a NUL-terminated integer representation. Use a base from
2–36 and `alpha = 'a'` or `'A'` for digits above 9. Decimal conversion handles
negative values by writing a leading minus; use nonnegative inputs for other
bases. The current negation-based implementation does not reliably handle the
minimum signed int. The caller must provide enough storage for the digits,
optional sign and terminator; a 16-bit binary value needs up to 17 bytes.

The formatted functions support only:

| Conversion | Argument |
|---|---|
| `%s` | Non-null pointer to a NUL-terminated string |
| `%d` | int, decimal |
| `%x` / `%X` | Nonnegative int, hexadecimal with lower/upper-case letters |

An optional decimal field width pads with spaces; `-` selects left alignment.
For example, `%5d` and `%-8s` work. Leading zeroes in a width do not select zero
padding. Precision, length modifiers, `%u`, `%c`, floating point and `%%` are
not implemented. The same integer-conversion limits apply to numeric formats.

`zgdk_sprintf` NUL-terminates the caller's buffer and returns the number of
characters excluding the terminator. It has no capacity parameter or bounds
check. `zgdk_printf` writes to `DEV_STDOUT`; `zgdk_fprintf` writes to a Zeal OS
device handle, not a C FILE pointer. Both use a 256-byte stack buffer: keep the
entire formatted result, including padding, to at most 255 characters. They
return void and do not propagate write errors.

```c
#include <zgdk/utils/string.h>

char message[32];
zgdk_sprintf(message, "SCORE %5d", 120);
zgdk_printf("%s\n", message);
zgdk_fprintf(DEV_STDOUT, "LEVEL %d\n", 3);
```

### String length

The SDK's separate `<zgdk/utils/_strlen.h>` declares
`uint16_t zgdk_strlen(const char *str)` and defines `strlen` as an alias for it.
It counts bytes before the NUL terminator. Despite the directory grouping,
`<zgdk/utils/string.h>` does not include this header. Include it explicitly if
the SDK length helper is intended, or use the standard string header for the
standard-library function.

## Console logging

Include `<zgdk/utils/log.h>`.

```c
void log_error(const char *message, uint8_t err);
void log_str(const char *message, const char *value);
```

These helpers use coreutils console output without the formatted-string buffer.
`log_error("load", 5)` prints `load 5 (0x05)` followed by a newline.
`log_str("file: ", "save.bin")` prints `file: save.bin` followed by a newline;
no separator is inserted between its two arguments.

## Emulator T-state markers

Include `<zgdk/utils/debug.h>`. With `EMULATOR` defined,
`TSTATE_LOG(counter)` writes the marker value to I/O port `0x86`, the emulator's
debug register. Diagnostic output depends on emulator support for that port;
the macro itself does not read a timer or return elapsed time.

```c
TSTATE_LOG(0);
/* Work to measure. */
TSTATE_LOG(1);
```

Without `EMULATOR`, the macro expands to nothing and does not evaluate its
argument. Keep marker expressions free of side effects.

## Parent directories

Include `<zgdk/utils/ensure_parent_dirs.h>` explicitly.

```c
zos_err_t ensure_parent_dirs(const char *path);
```

Given a file path such as `B:/saves/profile/save.dat`, this attempts to create
`B:/saves` and `B:/saves/profile`; it does not create `save.dat`. The final
component is treated as a filename unless followed by a slash. Absolute-root
and drive-root prefixes are skipped when creating directories.

The function accepts `ERR_ALREADY_EXIST` from mkdir, propagates other mkdir
errors, and returns `ERR_PATH_TOO_LONG` when the path will not fit its `PATH_MAX`
buffer including NUL. It returns `ERR_SUCCESS` after processing all parents.
Check the result before creating or opening the file. See [File](File.md) for
reading file data safely across memory pages.

## Byte-grid row addresses

Include `<zgdk/utils/grid_rows.h>` (also included by `<zgdk/utils.h>`).

```c
uint8_t cells[20 * 15];
const uint8_t *rows[15];

grid_rows_init(rows, cells, 20, 15);
uint8_t tile = rows[y][x];  /* caller validates x and y */
```

`grid_rows_init(const uint8_t **rows, const void *data, uint16_t stride,
uint8_t count)` initializes row addresses for a byte grid. `stride` is the
number of bytes between row starts and may include padding. `count` is 0–255.
The caller owns the address table and backing data: reserve at least `count`
entries and an addressable backing buffer containing `count` rows of `stride`
bytes. A zero count leaves the table untouched and permits null pointers.
A zero stride makes every row refer to the same address.

The helper allocates nothing, copies no grid data, and has no tile-property,
collision, graphics or global-map dependencies. Changes to cells remain visible
through the row pointers. Reinitialize when the base address or stride changes.
Row pointers take two bytes each on Z80, the same as 16-bit row offsets, and
avoid adding the buffer base on each lookup. Independent buffers use independent
tables. Callers interpret the bytes as tile IDs, text glyphs, terrain or other
grid data; the helper assigns no meanings to values.

This function is header-only and uses `INLINE` from `<zgdk/inline.h>`. No rebuilt `zgdk.lib` is
required, but consumers need these updated headers. Do not take the function's
address on SDCC: no out-of-line definition is provided.

Run `sh tests/run-grid-rows.sh` for the standalone ASan/UBSan tests. The suite
covers padded rows, strides above 255 bytes, 255 rows, high-byte values, zero
count/stride, table rebinding and backing-data edits.

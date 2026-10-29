# ZGDK: Chunked Tilesets

The Video Board loaders (`gfx_tileset_load_lz()`, `_rle()`, `_none()`, ...) decode a tileset from one buffer in RAM.
A tileset that isn't embedded in the program (it's read from a file or an archive) would need a buffer as large as
the whole tileset, just once, at load time.

A **chunked tileset** (`.tch`) cuts the tileset into chunks of whole tiles. `tileset_load_chunks()` streams it
chunk by chunk through a buffer that only has to hold the largest chunk, and any RAM the game uses later
(a level or music buffer) can serve as that buffer.

## Making a .tch

With CMake (after `zgdk_init.cmake`):

```cmake
tileset_chunks(my_game GIF assets/tiles.gif FORMAT lz MAX 2048)
# -> ${CMAKE_BINARY_DIR}/assets/tiles.tch and tiles.ztp (the palette)
```

| Option | Meaning |
|---|---|
| `GIF` | Source GIF, converted with the Video Board SDK's `gif2zeal.py` |
| `FORMAT` | `lz` (default), `rle`, `none`, `4bit`, `2bit` or `1bit`: the tile data, and the loader you pass at run time |
| `MAX` | Largest chunk in bytes, so the size of your load buffer (default 2048) |
| `TILESET`, `PALETTE` | Output paths (default `BUILD_DIR/<gif name>.tch` and `.ztp`) |
| `BUILD_DIR` | Directory for generated files (default `${CMAKE_BINARY_DIR}/assets`) |
| `UNIQUE`, `COLORS`, `STRIP` | Passed to gif2zeal (`-u`, `-c`, `-s`) |

The outputs are ordinary files: put them in an archive, copy them next to the program, or embed them.

Or by hand, from any `.zts` that gif2zeal wrote:

```sh
gif2zeal.py -i tiles.gif -t tiles.zts -p tiles.ztp -z lz
tools/zts_chunks.py tiles.zts tiles.tch --format lz --max 2048
```

`zts_chunks.py` needs Python 3 with Pillow and the Video Board SDK (`--zvb`, default `$ZVB_SDK_PATH`). For `lz` and
`rle` it finds the tile boundaries with the SDK's own codec (`tools/zeal2gif`): it decodes the tileset, re-encodes
each tile, and fails unless the result matches the input byte for byte. `--format` must match how gif2zeal wrote
the file; a wrong format fails rather than producing a bad `.tch`.

## Loading a .tch

```c
#include "zgdk/tileset.h"

zos_err_t tileset_load_chunks(gfx_context *ctx, tileset_read_fn read, void *source, tileset_load_fn load,
                              void *buffer, uint16_t buffer_size, gfx_tileset_options *options);
```

- `read(source, buffer, &size)` supplies the bytes, with the same contract as ZOS `read()`: up to `*size` bytes,
  `*size` set to what was read. The data can come from a file, an archive entry, or memory.
- `load` is the Video Board loader for the format. Only that one is linked, unlike `gfx_tileset_load()`, which
  links every decoder.
- `buffer` must hold the largest chunk, the `MAX` the file was made with. A larger chunk returns `ERR_NO_MORE_MEMORY`.
- `options->from_byte` is where the first tile goes in tile memory; on return it is just past the last tile. Tiles are
  256 bytes in 8-bit modes and 128 in 4-bit modes. `pal_offset` and `opacity` are passed to `load`;
  `compression` is not used.

| FORMAT | Loader to pass | Video mode |
|---|---|---|
| `lz` | `gfx_tileset_load_lz` | 8-bit |
| `rle` | `gfx_tileset_load_rle` | 8-bit |
| `none` | `gfx_tileset_load_none` | 8-bit (or 4-bit tiles made with `4bit`) |
| `4bit`, `2bit` | `gfx_tileset_load_4bit`, `gfx_tileset_load_2bit` | 8-bit |
| `1bit` | a wrapper around `gfx_tileset_load_1bit`, which has no `opacity` argument | 8-bit |

From a file. ZOS `read()` buffers must stay inside one 16 KiB virtual page (`zos_vfs.h`), so read through
`file_read_safe()` ([File](File.md)), which splits each read at page boundaries; a buffer anywhere in RAM works.

```c
#include "zgdk/file.h"

static zos_err_t file_read(void *dev, void *buffer, uint16_t *size) {
    return file_read_safe(*(zos_dev_t *)dev, buffer, size);
}

zos_dev_t dev = open("tiles.tch", O_RDONLY);
gfx_tileset_options options = { 0 };
err = tileset_load_chunks(&vctx, file_read, &dev, gfx_tileset_load_lz, level_buffer, sizeof(level_buffer), &options);
close(dev);
```

From a zeal-archiver (`.zar`) entry (`zar_entry_read()` already splits reads at page boundaries):

```c
static zos_err_t entry_read(void *entry, void *buffer, uint16_t *size) {
    return zar_entry_read(&archive, (zar_entry_t *)entry, buffer, size);
}

zar_entry_t entry;
err = zar_entry_from_name(&archive, "tiles.tch", &entry);
if (err == ERR_SUCCESS)
    err = tileset_load_chunks(&vctx, entry_read, &entry, gfx_tileset_load_lz, buffer, sizeof(buffer), &options);
```

Reading stops right after the one-byte end marker, so other data can follow the tileset in the same file.

Tile memory is 64 KiB: 256 tiles in 8-bit modes, 512 in 4-bit modes. When the tiles end exactly at its end,
`from_byte` comes back as 0.

Errors: the read error as returned; `ERR_NO_MORE_MEMORY` for a chunk larger than the buffer; `ERR_INVALID_OFFSET`
for tiles that would go past the end of tile memory (that chunk isn't loaded, so nothing wraps around to tile 0);
`ERR_ENTRY_CORRUPTED` when the data ends early (a cut header or chunk, or no end marker); `ERR_FAILURE` when `load`
fails.

## Format

```
repeated:  u8  tiles      tiles in this chunk, 1-255
           u16 size       bytes of tile data, little endian
           size bytes     the tile data for those tiles, as gif2zeal wrote it
then:      u8  0          end
```

Every chunk is whole tiles, so each one loads on its own. gif2zeal compresses every tile separately, so RLE and LZ
data can be cut at any tile boundary; the file doesn't record the format, and the caller passes the matching loader.

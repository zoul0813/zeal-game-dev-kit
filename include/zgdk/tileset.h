#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>

#ifndef TILESET_H
#define TILESET_H

/**
 * Chunked tilesets (.tch, made by tools/zts_chunks.py or tileset_chunks() in CMake).
 *
 * The Video Board loaders decode a tileset from one buffer in RAM. A .tch file cuts the tileset into
 * chunks of whole tiles, so it can be streamed from a file or an archive through a small buffer
 * (the largest chunk) instead of loading the whole tileset into RAM first.
 *
 * Format, repeated:  u8 tiles, u16 size (little endian), size bytes of tile data
 *         then:      u8 0
 * The tile data is whatever the matching Video Board loader takes (LZ, RLE, raw, 4/2/1-bit);
 * the file doesn't say which, the caller passes the loader.
 */

/**
 * @brief Reads up to *size bytes from source into buffer; sets *size to the bytes read.
 * Same contract as ZOS read(): ERR_SUCCESS with fewer bytes (or 0) at the end of the data.
 * Like read(), it must not hand ZOS a buffer that crosses a 16 KiB page: for a file, call
 * file_read_safe() (zgdk/file.h); zar_entry_read() already does the same.
 */
typedef zos_err_t (*tileset_read_fn)(void *source, void *buffer, uint16_t *size);

/**
 * @brief A Video Board tileset loader: gfx_tileset_load_lz, _rle, _none, _4bit or _2bit.
 * gfx_tileset_load_1bit has no opacity argument; wrap it in a function of this type.
 */
typedef gfx_error (*tileset_load_fn)(gfx_context *ctx, uint8_t *tileset, uint16_t size, uint16_t from,
                                     uint8_t pal_offset, uint8_t opacity);

/**
 * @brief Stream a chunked tileset into tile memory.
 *
 * Reads chunk after chunk through read(source, ...) into buffer and passes each to load(), until the
 * end marker. Only the loader you pass is linked. options->from_byte is the VRAM byte offset of the
 * first tile; on return it is just past the last tile (256 bytes per tile in 8-bit modes, 128 in
 * 4-bit modes). options->compression is not used; pal_offset and opacity go to load().
 * Reads stop right after the end marker, so other data can follow the tileset in the same stream.
 * Tile memory is 64 KiB (256 tiles in 8-bit modes, 512 in 4-bit modes); from_byte is 0 on return when the
 * tiles end exactly at its end.
 *
 * @param buffer Holds one chunk at a time: at least the largest chunk (the MAX the file was made with).
 *
 * @return ERR_SUCCESS; the read error; ERR_NO_MORE_MEMORY for a chunk larger than buffer_size;
 *         ERR_INVALID_OFFSET for tiles past the end of tile memory (nothing of that chunk is loaded);
 *         ERR_ENTRY_CORRUPTED for data that ends early; ERR_FAILURE if load() fails.
 */
zos_err_t tileset_load_chunks(gfx_context *ctx, tileset_read_fn read, void *source, tileset_load_fn load,
                              void *buffer, uint16_t buffer_size, gfx_tileset_options *options);

#endif

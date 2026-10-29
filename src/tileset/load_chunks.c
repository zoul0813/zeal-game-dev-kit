#include "zgdk/tileset.h"

zos_err_t tileset_load_chunks(gfx_context *ctx, tileset_read_fn read, void *source, tileset_load_fn load,
                              void *buffer, uint16_t buffer_size, gfx_tileset_options *options)
{
    uint8_t tiles;
    uint8_t head[2];
    uint16_t size;
    uint16_t chunk;
    uint16_t bytes;
    uint16_t from = options->from_byte;
    // tile memory is 64 KiB: once the tiles reach its end, from wraps to 0 and nothing more fits
    uint8_t full = 0;
    // 8-bit modes hold 256-byte tiles (the 4/2/1-bit loaders expand to 8 bits); 4-bit modes 128
    uint8_t shift = ctx->bpp == 8 ? 8 : 7;
    zos_err_t err;

    while (1) {
        // the tile count alone first: the end marker is one byte, and data may follow it
        size = 1;
        err = read(source, &tiles, &size);
        if (err != ERR_SUCCESS) return err;
        // no end marker before the end of the data
        if (size == 0) return ERR_ENTRY_CORRUPTED;
        if (tiles == 0) break;
        size = 2;
        err = read(source, head, &size);
        if (err != ERR_SUCCESS) return err;
        if (size != 2) return ERR_ENTRY_CORRUPTED;

        chunk = head[0] | (uint16_t)head[1] << 8;
        if (chunk > buffer_size) return ERR_NO_MORE_MEMORY;
        // at most 255 tiles of 256 bytes: fits 16 bits. Room left is 0x10000 - from (all of it when from is 0)
        bytes = (uint16_t)tiles << shift;
        if (full || (from != 0 && bytes > (uint16_t)(0 - from))) return ERR_INVALID_OFFSET;
        size = chunk;
        err = read(source, buffer, &size);
        if (err != ERR_SUCCESS) return err;
        if (size != chunk) return ERR_ENTRY_CORRUPTED;

        if (load(ctx, buffer, chunk, from, options->pal_offset, options->opacity) != GFX_SUCCESS) return ERR_FAILURE;
        from += bytes;
        if (from == 0) full = 1;
    }
    // 0 when the tiles end exactly at the end of tile memory
    options->from_byte = from;
    return ERR_SUCCESS;
}

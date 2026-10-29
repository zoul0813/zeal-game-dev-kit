/* Host test for tileset_load_chunks(): a .tch in memory, a recording loader. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/tileset.h"

static uint8_t tch[96];
static uint16_t tch_len, cursor;
static int fail_read_at = -1, reads;

static zos_err_t mem_read(void *source, void *buffer, uint16_t *size)
{
    (void) source;
    if (reads++ == fail_read_at) return ERR_FAILURE;
    if (*size > tch_len - cursor) *size = tch_len - cursor;
    memcpy(buffer, tch + cursor, *size);
    cursor += *size;
    return ERR_SUCCESS;
}

static struct { uint16_t size, from; uint8_t pal_offset, opacity, first; } calls[8];
static int ncalls, fail_load;

static gfx_error rec_load(gfx_context *ctx, uint8_t *tileset, uint16_t size, uint16_t from, uint8_t pal_offset,
                          uint8_t opacity)
{
    (void) ctx;
    calls[ncalls].size = size;
    calls[ncalls].from = from;
    calls[ncalls].pal_offset = pal_offset;
    calls[ncalls].opacity = opacity;
    calls[ncalls].first = tileset[0];
    ncalls++;
    return fail_load ? GFX_INVALID_ARG : GFX_SUCCESS;
}

static void chunk(uint8_t tiles, uint16_t size, uint8_t fill)
{
    tch[tch_len++] = tiles;
    tch[tch_len++] = size & 0xff;
    tch[tch_len++] = size >> 8;
    memset(tch + tch_len, fill, size);
    tch_len += size;
}

/* Two chunks: 3 tiles in 5 bytes, 2 tiles in 4 bytes, then the end marker */
static void build(int end_marker)
{
    tch_len = 0;
    chunk(3, 5, 0xA1);
    chunk(2, 4, 0xB2);
    if (end_marker) tch[tch_len++] = 0;
}

static zos_err_t run(uint8_t bpp, uint16_t buffer_size, gfx_tileset_options *options)
{
    gfx_context ctx = { 0, bpp };
    uint8_t buffer[16];
    cursor = 0;
    reads = 0;
    ncalls = 0;
    return tileset_load_chunks(&ctx, mem_read, NULL, rec_load, buffer, buffer_size, options);
}

int main(void)
{
    gfx_tileset_options options = { 0, 512, 7, 1 };

    // 8-bit mode: 256 bytes per tile in VRAM
    build(1);
    assert(run(8, 16, &options) == ERR_SUCCESS);
    assert(ncalls == 2);
    assert(calls[0].size == 5 && calls[0].from == 512 && calls[0].first == 0xA1);
    assert(calls[0].pal_offset == 7 && calls[0].opacity == 1);
    assert(calls[1].size == 4 && calls[1].from == 512 + 3 * 256 && calls[1].first == 0xB2);
    assert(options.from_byte == 512 + 5 * 256);
    assert(cursor == tch_len);

    // 4-bit mode: 128 bytes per tile
    options.from_byte = 0;
    assert(run(4, 16, &options) == ERR_SUCCESS);
    assert(calls[1].from == 3 * 128 && options.from_byte == 5 * 128);

    // a buffer exactly the largest chunk is enough; one byte less is not
    options.from_byte = 0;
    assert(run(8, 5, &options) == ERR_SUCCESS);
    options.from_byte = 0;
    assert(run(8, 4, &options) == ERR_NO_MORE_MEMORY && ncalls == 0 && options.from_byte == 0);

    // the data ends without the end marker
    build(0);
    options.from_byte = 0;
    assert(run(8, 16, &options) == ERR_ENTRY_CORRUPTED && ncalls == 2 && options.from_byte == 0);

    // a header cut short, and a chunk cut short
    build(1);
    tch_len = 5 + 3 + 1;   // first chunk, then 1 byte of the second header
    assert(run(8, 16, &options) == ERR_ENTRY_CORRUPTED && ncalls == 1);
    tch_len = 3 + 2;       // the first chunk's data ends after 2 of 5 bytes
    assert(run(8, 16, &options) == ERR_ENTRY_CORRUPTED && ncalls == 0);

    // read and load errors come back (reads: tile count, size, data, ...)
    build(1);
    fail_read_at = 2;      // the first chunk's data
    assert(run(8, 16, &options) == ERR_FAILURE && ncalls == 0);
    fail_read_at = -1;
    fail_load = 1;
    assert(run(8, 16, &options) == ERR_FAILURE && ncalls == 1);
    fail_load = 0;

    // a failing read of the size bytes
    fail_read_at = 1;
    assert(run(8, 16, &options) == ERR_FAILURE && ncalls == 0);
    fail_read_at = -1;

    // the end marker is one byte: data after it stays unread
    build(1);
    tch[tch_len++] = 0xEE;
    tch[tch_len++] = 0xEF;
    options.from_byte = 0;
    assert(run(8, 16, &options) == ERR_SUCCESS && cursor == tch_len - 2);

    // tile memory is 64 KiB: tiles ending exactly at its end load, one tile more is refused, nothing wraps
    build(1);
    options.from_byte = 0x10000 - 5 * 256;
    assert(run(8, 16, &options) == ERR_SUCCESS && ncalls == 2 && options.from_byte == 0);
    options.from_byte = 0x10000 - 4 * 256;
    assert(run(8, 16, &options) == ERR_INVALID_OFFSET && ncalls == 1 && options.from_byte == 0x10000 - 4 * 256);
    options.from_byte = 0x10000 - 3 * 256;   // the first chunk ends exactly at the end: the second can't wrap to 0
    assert(run(8, 16, &options) == ERR_INVALID_OFFSET && ncalls == 1 && calls[0].from == 0x10000 - 3 * 256);
    options.from_byte = 0x10000 - 2 * 256;   // not even the first chunk fits
    assert(run(8, 16, &options) == ERR_INVALID_OFFSET && ncalls == 0);
    options.from_byte = 0x10000 - 5 * 128;   // 4-bit modes: 128 bytes per tile
    assert(run(4, 16, &options) == ERR_SUCCESS && options.from_byte == 0);
    options.from_byte = 0x10000 - 4 * 128;
    assert(run(4, 16, &options) == ERR_INVALID_OFFSET && ncalls == 1);

    // only the end marker: nothing loaded, from_byte unchanged
    tch_len = 0;
    tch[tch_len++] = 0;
    options.from_byte = 1024;
    assert(run(8, 16, &options) == ERR_SUCCESS && ncalls == 0 && options.from_byte == 1024);

    puts("tileset: ok");
    return 0;
}

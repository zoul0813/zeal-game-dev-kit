#!/usr/bin/env python3
"""Split a Zeal tileset (.zts from the Video Board SDK's gif2zeal.py) into chunks that load one at a time.

The Video Board loaders decode a tileset from one buffer in RAM. gif2zeal writes every tile on its own
(RLE and LZ compress each tile separately), so the data can be cut at any tile boundary and each piece
still loads alone. This packs whole tiles into chunks of at most --max bytes, so a game only needs a
--max byte buffer to stream its tileset from a file or archive (tileset_load_chunks() in zgdk/tileset.h).

Output (.tch):
    repeated: u8 tiles, u16 size (little endian), size bytes of tile data
    then:     u8 0

--format is the .zts data, as gif2zeal wrote it:
    lz, rle    gif2zeal -z lz / -z rle (8-bit tiles; the Video Board decoders need an 8-bit mode)
    none       uncompressed 8-bit tiles, 256 bytes each
    4bit, 2bit, 1bit    gif2zeal -b 4 / -b 2 / -b 1, 128 / 64 / 32 bytes per tile
For lz and rle the tile boundaries come from the Video Board SDK's own codec (tools/zeal2gif): the
tileset is decoded, each tile re-encoded, and the result must match the input byte for byte.

Usage: zts_chunks.py tileset.zts tiles.tch --format lz --max 2048 [--zvb $ZVB_SDK_PATH]
"""
import argparse
import os
import struct
import sys

TILE = 256
FIXED = {'none': 256, '4bit': 128, '2bit': 64, '1bit': 32}


def zvb_codec(zvb_path, fmt):
    """(decode, encode_tile) from the Video Board SDK's zeal2gif/gif2zeal scripts."""
    tools = os.path.join(zvb_path, 'tools', 'zeal2gif')
    if not os.path.isfile(os.path.join(tools, 'gif2zeal.py')):
        sys.exit('zts_chunks: no tools/zeal2gif/gif2zeal.py under %r (set --zvb or ZVB_SDK_PATH)' % zvb_path)
    sys.path.insert(0, tools)
    sys.dont_write_bytecode = True   # leave no __pycache__ in the SDK
    import gif2zeal
    import zeal2gif
    if fmt == 'lz':
        return zeal2gif.decompress_lz, gif2zeal.compress_lz
    return zeal2gif.decompress, gif2zeal.compress


def tile_sizes(data, fmt, zvb_path):
    """Bytes of each tile in data."""
    if fmt in FIXED:
        size = FIXED[fmt]
        if len(data) % size:
            sys.exit('zts_chunks: %d bytes is not a whole number of %d-byte tiles (--format %s)' % (len(data), size, fmt))
        return [size] * (len(data) // size)

    decode, encode = zvb_codec(zvb_path, fmt)
    raw = bytes(decode(data))
    if len(raw) % TILE:
        sys.exit('zts_chunks: decodes to %d bytes, not whole 256-byte tiles; is it --format %s, 8-bit?' % (len(raw), fmt))
    sizes, pos = [], 0
    for i in range(0, len(raw), TILE):
        tile = bytes(encode(list(raw[i:i + TILE])))
        if data[pos:pos + len(tile)] != tile:
            sys.exit('zts_chunks: tile %d does not re-encode to the input; not gif2zeal %s output, or the codec changed'
                     % (i // TILE, fmt))
        sizes.append(len(tile))
        pos += len(tile)
    if pos != len(data):
        sys.exit('zts_chunks: %d bytes left after the last tile' % (len(data) - pos))
    return sizes


def chunks(sizes, limit):
    """[(tiles, bytes)] of whole tiles, at most limit bytes and 255 tiles each."""
    out, first = [], 0
    while first < len(sizes):
        if sizes[first] > limit:
            sys.exit('zts_chunks: tile %d alone is %d bytes, more than --max %d' % (first, sizes[first], limit))
        last, total = first, sizes[first]
        while last + 1 < len(sizes) and last + 1 - first < 255 and total + sizes[last + 1] <= limit:
            last += 1
            total += sizes[last]
        out.append((last - first + 1, total))
        first = last + 1
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('input', help='.zts from gif2zeal')
    ap.add_argument('output', help='.tch to write')
    ap.add_argument('--format', required=True, choices=['lz', 'rle'] + list(FIXED), help='how gif2zeal wrote the .zts')
    ap.add_argument('--max', type=int, default=2048, help="largest chunk in bytes (the game's load buffer)")
    ap.add_argument('--zvb', default=os.environ.get('ZVB_SDK_PATH', ''), help='Video Board SDK (default $ZVB_SDK_PATH)')
    a = ap.parse_args()
    if not 1 <= a.max <= 0xFFFF:
        sys.exit('zts_chunks: --max must be 1-65535')

    data = open(a.input, 'rb').read()
    sizes = tile_sizes(data, a.format, a.zvb)
    pos = 0
    with open(a.output, 'wb') as f:
        parts = chunks(sizes, a.max)
        for tiles, size in parts:
            f.write(struct.pack('<BH', tiles, size) + data[pos:pos + size])
            pos += size
        f.write(b'\0')
    print('zts_chunks: %d tiles, %d bytes %s -> %d chunks (largest %d, limit %d)' % (
        len(sizes), len(data), a.format, len(parts), max((s for _, s in parts), default=0), a.max))


if __name__ == '__main__':
    main()

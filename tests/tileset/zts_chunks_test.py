#!/usr/bin/env python3
"""Test tools/zts_chunks.py against the Video Board SDK's gif2zeal, for every format.

Needs Python 3 with Pillow and the SDK: ZVB_SDK_PATH (or the first argument).
"""
from pathlib import Path
import os
import struct
import subprocess
import sys
import tempfile

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
ZVB = Path(sys.argv[1] if len(sys.argv) > 1 else os.environ.get('ZVB_SDK_PATH', ''))
GIF2ZEAL = ZVB / 'tools' / 'zeal2gif' / 'gif2zeal.py'
CHUNKS = ROOT / 'tools' / 'zts_chunks.py'
TILES = 48
# format: (gif2zeal options, bytes per tile when fixed)
FORMATS = {'lz': (['-z', 'lz'], None), 'rle': (['-z', 'rle'], None), 'none': ([], 256),
           '4bit': (['-b', '4'], 128), '2bit': (['-b', '2'], 64), '1bit': (['-b', '1'], 32)}


def make_gif(path):
    """TILES 16x16 tiles in 16 colors: flat ones, stripes and noise, so the codecs see runs and literals."""
    img = Image.new('P', (16 * 8, 16 * (TILES // 8)))
    img.putpalette([c for i in range(16) for c in (i * 16, 255 - i * 16, (i * 37) & 255)])
    seed = 1
    for t in range(TILES):
        ox, oy = (t % 8) * 16, (t // 8) * 16
        for y in range(16):
            for x in range(16):
                if t % 3 == 0:
                    c = t % 16
                elif t % 3 == 1:
                    c = (x // (1 + t % 4) + y) % 16
                else:
                    seed = (seed * 75 + 74) % 65537
                    c = seed % 16
                img.putpixel((ox + x, oy + y), c)
    img.save(path)


def run(*cmd, ok=True):
    p = subprocess.run([sys.executable, *map(str, cmd)], capture_output=True, text=True)
    if ok and p.returncode:
        sys.exit('failed: %s\n%s%s' % (' '.join(map(str, cmd)), p.stdout, p.stderr))
    return p


def parse(tch):
    """[(tiles, data)] and checks the end marker."""
    out, pos = [], 0
    while tch[pos]:
        tiles, size = struct.unpack_from('<BH', tch, pos)
        out.append((tiles, tch[pos + 3:pos + 3 + size]))
        pos += 3 + size
    assert pos == len(tch) - 1, 'data after the end marker'
    return out


def main():
    if not GIF2ZEAL.is_file():
        sys.exit('set ZVB_SDK_PATH (no %s)' % GIF2ZEAL)
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        gif = tmp / 'tiles.gif'
        make_gif(gif)
        zts = {}
        for fmt, (opts, fixed) in FORMATS.items():
            zts[fmt] = tmp / (fmt + '.zts')
            run(GIF2ZEAL, '-i', gif, '-t', zts[fmt], '-p', tmp / 'tiles.ztp', '-o', tmp, *opts)
            data = zts[fmt].read_bytes()
            for limit in (300, 1000, 65535):
                tch = tmp / (fmt + '.tch')
                run(CHUNKS, zts[fmt], tch, '--format', fmt, '--max', limit, '--zvb', ZVB)
                parts = parse(tch.read_bytes())
                assert b''.join(d for _, d in parts) == data, (fmt, limit, 'data changed')
                assert sum(t for t, _ in parts) == TILES, (fmt, limit, 'tile count')
                assert all(len(d) <= limit and 0 < t <= 255 for t, d in parts), (fmt, limit, 'chunk limits')
                if fixed:
                    assert all(len(d) == t * fixed for t, d in parts), (fmt, limit, 'whole tiles')
            print('%-5s %5d bytes ok' % (fmt, len(data)))

        # wrong format, a limit below one tile, a truncated file: all fail
        bad = tmp / 'bad.tch'
        assert run(CHUNKS, zts['rle'], bad, '--format', 'lz', '--zvb', ZVB, ok=False).returncode
        assert run(CHUNKS, zts['lz'], bad, '--format', 'rle', '--zvb', ZVB, ok=False).returncode
        assert run(CHUNKS, zts['none'], bad, '--format', 'none', '--max', 255, ok=False).returncode
        cut = tmp / 'cut.zts'
        cut.write_bytes(zts['lz'].read_bytes()[:-1])
        assert run(CHUNKS, cut, bad, '--format', 'lz', '--zvb', ZVB, ok=False).returncode
        cut.write_bytes(zts['4bit'].read_bytes()[:-1])
        assert run(CHUNKS, cut, bad, '--format', '4bit', ok=False).returncode
        print('errors ok')


if __name__ == '__main__':
    main()

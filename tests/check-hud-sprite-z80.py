#!/usr/bin/env python3
"""Execute compiled HUD code; host C tests cannot detect SDCC pointer drift.
Usage: python3 tests/check-hud-sprite-z80.py fixture.ihx (requires z80).
"""
from pathlib import Path
import re
import struct
import sys
import z80

path = Path(sys.argv[1])
text = path.with_suffix('.map').read_text()
symbols = {name: int(address, 16) for address, name in re.findall(r"\b([0-9A-F]{8})\s+(_\w+)", text)}
for caption in (b'TIDEPUP', b'ROOFTOP ESCAPE', b'OUTRUN THE STORM', b'a SWIM KICK  b SLIDE', b'PRESS g', b'A B', b''):
    for capacity in (1, 5, 12, 20):
        machine = z80.Z80Machine()
        for line in path.read_text().splitlines():
            record = bytes.fromhex(line[1:])
            assert sum(record) % 256 == 0
            if record[3] == 0:
                machine.set_memory_block(int.from_bytes(record[1:3], 'big'), record[4:-1])
        # No CRT is executed; initialize the shared mapping and fixture explicitly.
        mapping = list(range(32,127))
        for character, tile in ((ord('a'),200),(ord('b'),201),(ord('g'),202)):
            mapping[character-32] = tile
        machine.set_memory_block(symbols['_ASCII_MAP'], bytes(mapping))
        machine.set_memory_block(0x2000, caption + b'\0')
        machine.set_memory_block(symbols['_test_text'], struct.pack('<H',0x2000))
        machine.set_memory_block(symbols['_test_capacity'], bytes([capacity]))
        count = min(len(caption),capacity)
        machine.set_memory_block(symbols['_test_count'], bytes([count]))
        machine.set_memory_block(symbols['_test_sprites'], bytes([0x5a]) * 160)
        machine.set_memory_block(0xfd00, bytes(256))
        machine.set_memory_block(0xfe00, struct.pack('<H',0xff00))
        machine.set_memory_block(0xff00,b'\x76')
        machine.set_breakpoint(0xff00)
        machine.sp, machine.pc = 0xfe00, symbols['_main']
        machine.ticks_to_stop = 1000000
        while machine.pc != 0xff00:
            machine.run()
            assert machine.ticks_to_stop, 'runaway HUD call'
        assert machine.sp == 0xfe02
        count = min(len(caption),capacity)
        left = 174-count*6
        for i in range(capacity):
            at = symbols['_test_sprites']+8*i
            y,x,tile,flags,options = struct.unpack('<HHBBH',bytes(machine.memory[at:at+8]))
            if i < count and caption[i] > 32:
                assert (y,x,tile) == (136,left+i*12,mapping[caption[i]-32]), (caption,capacity,i,(y,x,tile))
            else:
                assert y >= 496, (caption,capacity,i,'not hidden',y)
            assert flags == 0x5a and options == 0x5a5a
        expected = bytes(mapping[c-32] for c in caption[:count])
        start = symbols['_test_tiles']
        assert bytes(machine.memory[start:start+count]) == expected, (caption,capacity,'tilemap')
        at=symbols['_test_sprites']+capacity*8
        assert bytes(machine.memory[at:symbols['_test_sprites']+160]) == bytes([0x5a])*(160-capacity*8)
print('Z80 HUD: full captions, shared icon mapping, truncation, centering, spaces and metadata passed (28 cases)')

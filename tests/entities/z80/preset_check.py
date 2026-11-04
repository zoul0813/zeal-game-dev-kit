#!/usr/bin/env python3
"""Exercise SDK entity preset ASM in a linked SDCC .ihx (adjacent .map required).

Requires Python's z80 package. No rendering substitute is compiled or linked.
An optional second .ihx compares sprite sync cycle costs against an earlier build.
"""
import re
import struct
import sys
from pathlib import Path

import z80

HALT = 0xFF00
RECORDS, SPRITES, ANIM, FRAMES = 0x1000, 0x2000, 0x2200, 0x2210


def load(path):
    machine = z80.Z80Machine()
    for line in Path(path).read_text().splitlines():
        data = bytes.fromhex(line[1:])
        assert sum(data) % 256 == 0, 'invalid Intel HEX checksum'
        if data[3] == 0:
            machine.set_memory_block(int.from_bytes(data[1:3], 'big'), data[4:-1])
        else:
            assert data[3] == 1, 'unsupported Intel HEX record'
    symbols = {}
    for address, name in re.findall(r'\b([0-9A-F]{8})\s+(_\w+)', Path(path).with_suffix('.map').read_text()):
        symbols[name] = int(address, 16)
    machine.set_memory_block(HALT, b'\x76')
    machine.set_breakpoint(HALT)
    # Standalone SDK fixtures have no CRT: initialize globals as startup does.
    text = Path(path).with_suffix('.map').read_text()
    areas = {k: (int(a, 16), int(n, 16)) for k, a, n in
             re.findall(r'^\s*(_(?:DATA|BSS|INITIALIZED|INITIALIZER))\s+([0-9A-F]+)\s+([0-9A-F]+)', text, re.M)}
    for k in ('_DATA', '_BSS'):
        if k in areas:
            a, n = areas[k]
            machine.set_memory_block(a, bytes(n))
    if '_INITIALIZED' in areas and '_INITIALIZER' in areas:
        dest, n = areas['_INITIALIZED']
        origin, _ = areas['_INITIALIZER']
        machine.set_memory_block(dest, bytes(machine.memory[origin:origin+n]))
    return machine, symbols


def word(machine, address, value):
    machine.set_memory_block(address, struct.pack('<H', value & 65535))


def call(machine, symbols, name, hl=0, de=0, stack=()):
    machine.sp = 0xFE00
    word(machine, machine.sp, HALT)
    for i, value in enumerate(stack):
        word(machine, machine.sp + 2 + i * 2, value)
    machine.hl, machine.de, machine.pc = hl & 65535, de & 65535, symbols[name]
    machine.ticks_to_stop = 1000000
    while machine.pc != HALT:
        machine.run()
        assert machine.ticks_to_stop, 'runaway Z80 call'
    assert machine.sp == 0xFE02 + len(stack) * 2, 'unbalanced stack'
    return 1000000 - machine.ticks_to_stop


def fixture(path, stride=32):
    machine, symbols = load(path)
    preset = '_physics_entities_sync' in symbols
    current = '_game_entities' in symbols
    retained = preset or '_game_entity_anim_play' in symbols
    pool = symbols.get('_test_pool', symbols.get('_game_entities'))
    if '_HEADER' in Path(path).with_suffix('.map').read_text():
        RECORDS, SPRITES, ANIM, FRAMES = 0x1000, 0x2000, 0x2200, 0x2210
    else:
        RECORDS, SPRITES, ANIM, FRAMES = 0x8000, 0x9000, 0x9200, 0x9210
    tag, flip_at, tall_at, hidden_at, ox_at, oy_at, parked = (2,3,4,5,6,7,8) if preset else (22,23,24,25,26,27,31)
    body, anim = (9,21) if preset else (2,17)
    view_fn = '_entity_view_set' if preset else ('_game_entities_set_view' if current else '_entities_set_view')
    sync_fn = '_physics_entities_sync' if preset else ('_game_entities_sync' if current else '_entities_sync')
    sync_arg = pool if preset else 0
    play_fn = '_animated_physics_entity_play' if preset else '_game_entity_anim_play'
    restart_fn = '_animated_physics_entity_restart' if preset else '_game_entity_anim_restart'
    if preset or current:
        machine.set_memory_block(pool, struct.pack('<HHBBB', RECORDS, RECORDS + stride, 8, stride, tag))
    else:
        machine.set_memory_block(symbols['_POOL'], struct.pack('<HBBH', RECORDS, 8, 1, RECORDS + stride))
    machine.set_memory_block(ANIM, struct.pack('<HBBB', FRAMES, 3, 1, 1))
    machine.set_memory_block(FRAMES, bytes([42, 43, 44]))
    times = []
    for k in range(4000):
        vx, vy = (k * 37) % 1600, (k * 53) % 1600
        px = max(0, vx + ((k * 7) % 400) - 40)
        py = max(0, vy + ((k * 11) % 330) - 45)
        x, y = px * 16 + k % 16, py * 16
        tile = (k * 29 + 173) & 255
        flags = (k * 13) & 255
        flip, tall, hidden = (k & 3) << 2, k & 1, k % 17 == 0
        free = k % 19 == 0
        record = bytearray(stride)
        struct.pack_into('<H', record, 0, SPRITES)
        struct.pack_into('<hhhhBBBB', record, body, x, y, 0, 0, 12, 30, 0, 0)
        struct.pack_into('<H', record, anim, ANIM if k & 2 else 0)
        record[anim + 2] = k % 3
        record[tag] = 255 if free else 1
        record[flip_at], record[tall_at], record[hidden_at] = flip, tall, hidden
        record[ox_at] = record[oy_at] = 254
        machine.set_memory_block(RECORDS, record)
        machine.set_memory_block(SPRITES, struct.pack('<HHBBBB', 100, 100, tile, flags, 2, 0))
        view_ticks = call(machine, symbols, view_fn, vx, vy)
        times.append(view_ticks + call(machine, symbols, sync_fn, sync_arg))
        sprite = bytes(machine.memory[SPRITES:SPRITES + 8])
        sx, sy = px - 2 - vx, py - 2 - vy
        top_visible = sy >= -16 if preset and tall else sy > -16
        visible = not (free or hidden) and -16 < sx < 320 and top_visible and sy < 240
        if visible:
            expected = struct.pack('<HHBBBB', sy + 16, sx + 16, tile if retained else ((42 + k % 3) if k & 2 else 0),
                                   (flags & 0xF3) | flip, 2 if tall else 0, 0)
            assert sprite == expected, (k, sprite, expected)
            assert machine.memory[RECORDS + parked] == 0
        else:
            assert sprite == struct.pack('<HHBBBB', 0, 100, tile, flags, 0, 0), (k, sprite)
            assert machine.memory[RECORDS + parked] == 1
        # Already parked records must remain untouched, including sprite options.
        if not visible:
            machine.set_memory_block(SPRITES, bytes([0xA5]) * 8)
            call(machine, symbols, sync_fn, sync_arg)
            assert bytes(machine.memory[SPRITES:SPRITES+8]) == bytes([0xA5]) * 8
    # Clear retains the high-water mark so formerly visible slots are parked.
    machine.set_memory_block(RECORDS + parked, b'\0')
    call(machine, symbols, '_entity_pool_clear' if preset or current else '_entities_clear',
         pool if preset or current else 0)
    call(machine, symbols, sync_fn, sync_arg)
    assert machine.memory[RECORDS + tag] == 255
    assert machine.memory[SPRITES] == machine.memory[SPRITES+1] == 0
    assert machine.memory[SPRITES+6] == machine.memory[SPRITES+7] == 0
    # Mixed live/free/hidden records exercise the stride and high-water boundary.
    end_address = pool + 2 if preset or current else symbols['_POOL'] + 4
    word(machine, end_address, RECORDS + 8 * stride)
    call(machine, symbols, view_fn, 0, 0)
    for i in range(8):
        record = bytearray(stride)
        struct.pack_into('<H', record, 0, SPRITES + i * 8)
        struct.pack_into('<hh', record, body, 16 * (20 + i), 16 * 30)
        record[tag] = 255 if i % 3 == 0 else 1
        record[hidden_at] = i % 3 == 1
        machine.set_memory_block(RECORDS + i * stride, record)
        machine.set_memory_block(SPRITES + i * 8, bytes(8))
    call(machine, symbols, sync_fn, sync_arg)
    for i in range(8):
        y, x = struct.unpack('<HH', bytes(machine.memory[SPRITES + i*8:SPRITES + i*8+4]))
        assert (y, x) == ((46, 36+i) if i % 3 == 2 else (0, 0))
    if retained:
        # Real SDK tick + game-side selection + production ASM sync. The caller
        # stores the returned byte just as each game update does.
        word(machine, pool + 2, RECORDS + stride)
        for rate, looping in ((1, 1), (2, 1), (1, 0)):
            record = bytearray(stride)
            struct.pack_into('<H', record, 0, SPRITES)
            struct.pack_into('<hh', record, body, 20 * 16, 30 * 16)
            record[tag] = 1
            machine.set_memory_block(RECORDS, record)
            machine.set_memory_block(ANIM, struct.pack('<HBBB', FRAMES, 3, rate, looping))
            machine.set_memory_block(FRAMES, bytes([201, 42, 255]))
            machine.set_memory_block(SPRITES, bytes(8))
            call(machine, symbols, play_fn, RECORDS, ANIM)
            assert machine.memory[SPRITES + 4] == 201  # Start before any tick.
            call(machine, symbols, sync_fn, sync_arg)
            assert machine.memory[SPRITES + 4] == 201
            for tick in range(3 * rate + 2):
                call(machine, symbols, play_fn, RECORDS, ANIM)
                call(machine, symbols, '_anim_tick', RECORDS + anim)
                got = machine.a
                index = (tick // rate) % 3 if looping else min(tick // rate, 2)
                expected = [201, 42, 255][index]
                assert got == expected, (rate, looping, tick, got, expected)
                machine.set_memory_block(SPRITES + 4, bytes([got]))
                # Same selection and repeated sync without another tick must
                # preserve the returned frame even after index has advanced.
                call(machine, symbols, play_fn, RECORDS, ANIM)
                call(machine, symbols, sync_fn, sync_arg)
                assert machine.memory[SPRITES + 4] == expected
                call(machine, symbols, sync_fn, sync_arg)
                assert machine.memory[SPRITES + 4] == expected
            call(machine, symbols, restart_fn, RECORDS, ANIM)
            assert machine.memory[SPRITES + 4] == 201
            assert bytes(machine.memory[RECORDS+anim+2:RECORDS+anim+5]) == bytes(3)
            call(machine, symbols, play_fn, RECORDS, 0)
            call(machine, symbols, sync_fn, sync_arg)
            assert machine.memory[SPRITES + 4] == 0
        print(f'{path}: retained-frame playback, rate-one/slow/one-shot, restart and null passed')
    print(f'{path}: 4,000 ASM sync cases, one-time parking and clear passed')
    return times


def edges(path, mode):
    """Check each production renderer against signed pixel bounds and hardware Y."""
    machine, symbols = load(path)
    record_address, sprite_address = 0x8000, 0x9000
    functions = {'sprite': '_sprite_entity_sync_fp', 'physics': '_physics_entity_sync',
                 'pool': '_physics_entities_sync'}
    if mode == 'pool':
        pool = symbols['_test_pool']
        machine.set_memory_block(pool, struct.pack('<HHBBB', record_address, record_address + 32, 1, 32, 2))
    cases = 0
    for width, height in ((320, 240), (640, 480)):
        word(machine, symbols['__entity_view_limit_x'], width + 15)
        word(machine, symbols['__entity_view_limit_y'], height + 15)
        for camera in (0, -1900, 1900):
            call(machine, symbols, '_entity_view_set', camera, camera)
            for tall in (0, 1):
                for axis, limit in ((0, width), (1, height)):
                    for offset in (-128, -2, 0, 2, 127):
                        # Include positions before applying the offset as well
                        # as exact final edges; positive offsets must recover
                        # negative origins on both axes. Y=-16 is tall-only.
                        origins = {-33, -32, -31, -17, -16, -15, 0,
                                   -17-offset, -16-offset, -15-offset,
                                   limit-1-offset, limit-offset, limit+1-offset}
                        for origin in sorted(origins):
                            for fraction in (0, 1, 15):
                                positions = [(camera + 20) * 16, (camera + 30) * 16]
                                positions[axis] = (camera + origin) * 16 + fraction
                                if not all(-32768 <= p <= 32767 for p in positions):
                                    continue
                                offsets = [0, 0]
                                offsets[axis] = offset
                                record = bytearray(32)
                                struct.pack_into('<H', record, 0, sprite_address)
                                record[2:8] = bytes([1, 8, tall, 0, offsets[0] & 255, offsets[1] & 255])
                                struct.pack_into('<hh', record, 9, *positions)
                                machine.set_memory_block(record_address, record)
                                machine.set_memory_block(sprite_address, struct.pack('<HHBBBB', 100, 100, 201, 0xF3, 2, 0))
                                if mode == 'sprite':
                                    call(machine, symbols, functions[mode], record_address, positions[0], (positions[1],))
                                else:
                                    call(machine, symbols, functions[mode], pool if mode == 'pool' else record_address)
                                sx, sy = [positions[i] // 16 - camera + offsets[i] for i in (0, 1)]
                                visible = -16 < sx < width and (-16 <= sy if tall else -16 < sy) and sy < height
                                expected = (struct.pack('<HHBBBB', sy + 16, sx + 16, 201, 0xFB, 2 if tall else 0, 0)
                                            if visible else struct.pack('<HHBBBB', 0, 100, 201, 0xF3, 0, 0))
                                actual = bytes(machine.memory[sprite_address:sprite_address+8])
                                assert actual == expected, (mode, width, height, camera, tall, axis, origin, offset, fraction, actual, expected)
                                assert machine.memory[record_address+8] == int(not visible)
                                cases += 1
    print(f'{mode}: {cases} signed-offset and tall-edge ASM cases passed')


def suite(directory):
    for kind in ('sprite', 'animated', 'physics', 'animated_physics', 'pool'):
        path = Path(directory) / (kind + '.ihx')
        machine, symbols = load(path)
        ticks = call(machine, symbols, '_main')
        assert machine.de == 0, (kind, machine.de)
        if kind in ('sprite', 'animated'):
            assert not any(re.match(r'_(?:body|physics|platformer)_', n) for n in symbols)
        if kind in ('sprite', 'physics'):
            assert not any(re.match(r'_(?:anim|animated)_', n) for n in symbols)
        if kind != 'pool':
            assert not any(re.match(r'_(?:tileflags|body_step|platformer)_', n) for n in symbols)
        print(f'{kind}: real Z80 behavior passed, optional dependencies absent ({ticks} T)')
        if kind in ('sprite', 'physics', 'pool'):
            edges(path, kind)
    for stride in (32, 37, 255):
        fixture(Path(directory) / 'pool.ihx', stride)


if __name__ == '__main__':
    if sys.argv[1] == '--suite':
        suite(sys.argv[2])
    else:
        after = fixture(sys.argv[1])
        if len(sys.argv) > 2:
            before = fixture(sys.argv[2])
            delta = [a - b for a, b in zip(after, before)]
            print(f'view + sync T-state delta: min {min(delta)}, max {max(delta)}')

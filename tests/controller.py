"""Execute linked SDCC C callers on a Z80 with a cycle-traced SNES user port.

Build: ZOS_PATH=... sh tests/build-controller.sh
Run: python tests/controller.py build/controller-tests [baseline-directory]
Test dependency: z80==1.2.0 (install in an isolated venv).
The optional baseline directory contains api/pad/mouse.{ihx,map,noi} linked against
the original C modules using the same callers and compiler options.
"""
import random
import re
import sys
from pathlib import Path

import z80


class Image:
    def __init__(self, path):
        self.path = Path(path)
        self.memory = bytearray(65536)
        for line in self.path.with_suffix('.ihx').read_text().splitlines():
            record = bytes.fromhex(line[1:])
            assert sum(record) & 255 == 0
            if record[3] == 0:
                addr = int.from_bytes(record[1:3], 'big')
                self.memory[addr:addr + record[0]] = record[4:4 + record[0]]
        self.map = self.path.with_suffix('.map').read_text()
        # .map truncates names at 32 characters, even with wide addresses.
        self.symbols = {name: int(addr, 16) for name, addr in re.findall(
            r'^DEF (\w+) 0x([0-9A-Fa-f]+)$', self.path.with_suffix('.noi').read_text(), re.M)}
        self.text_size = int(re.search(r'_TEXT\s+[0-9A-F]{8}\s+([0-9A-F]{8})\s+=', self.map)[1], 16)


class Port:
    def __init__(self, reports=(0, 0), mouse_port=None, speed=0, clone=False):
        self.reports = list(reports)
        self.mouse_port = mouse_port
        self.speed = speed
        self.clone = clone
        self.pins = 8
        self.index = 0
        self.cycles = 0
        self.writes = []
        self.reads = []
        self.m = None

    def time(self):
        return 10_000_000 - self.m.ticks_to_stop

    def output(self, address, value):
        self.writes.append((self.time(), address & 255, value))
        if address & 255 != 0xd0:
            return
        if value & 4 and not self.pins & 4:
            self.index = 0
        if self.pins & 8 and not value & 8:
            if value & 4:
                self.cycles += 1
                if not self.clone:
                    self.speed = (self.speed + 1) % 3
            else:
                self.index += 1
        self.pins = value

    def input(self, address):
        assert address & 255 == 0xd0
        self.reads.append((self.time(), self.index))
        value = self.pins & 0xfc
        for p, report in enumerate(self.reports):
            if p == self.mouse_port:
                report = (report & ~0x0c00) | (0, 0x0800, 0x0400)[self.speed]
            value |= (not (report >> self.index & 1)) << p
        return value

    def timing(self):
        """Check edge widths and settled sampling, including the final clock."""
        last = {4: None, 8: None}
        previous = 8
        latch_clocked = False
        for time, address, value in self.writes:
            if address != 0xd0:
                continue
            for bit in (4, 8):
                if (value ^ previous) & bit:
                    if last[bit] is not None:
                        width = time - last[bit]
                        if bit == 8:
                            assert width >= 60, ('clock pulse', width)
                        elif previous & 4:
                            if not latch_clocked:
                                assert width >= 120, ('latch pulse', width)
                    last[bit] = time
                    if bit == 4 and value & 4:
                        latch_clocked = False
                    if bit == 8 and value & 4:
                        latch_clocked = True
            previous = value
        assert self.pins == 8
        # Match every input to its most recent rising edge or latch release.
        for time, index in self.reads:
            edges = [t for t, a, v in self.writes if a == 0xd0 and v == 8 and t < time]
            if edges:
                minimum = 471 if index >= 16 else 60
                assert time - edges[-1] >= minimum, ('sample settling', index, time - edges[-1])


class Run:
    def __init__(self, image, port=None, preserve=True):
        self.image = image
        self.m = z80.Z80Machine()
        self.m.set_memory_block(0, image.memory)
        self.port = port or Port()
        self.port.m = self.m
        self.m.set_input_callback(self.port.input)
        self.m.set_output_callback(self.port.output)
        self.m.set_breakpoint(0)
        self.preserve = preserve

    def put(self, name, value, size=1):
        self.m.set_memory_block(self.image.symbols['_' + name], value.to_bytes(size, 'little'))

    def get(self, name, size=1):
        addr = self.image.symbols['_' + name]
        return int.from_bytes(self.m.memory[addr:addr + size], 'little')

    def call(self, name, **parameters):
        for key, value in parameters.items():
            self.put('test_' + key, value)
        self.m.ix, self.m.iy = 0x1234, 0x5678
        self.m.alt_af, self.m.alt_bc = 0x1111, 0x2222
        self.m.alt_de, self.m.alt_hl = 0x3333, 0x4444
        self.m.sp = 0xf000
        self.m.set_memory_block(0xf000, b'\0\0')
        target = self.image.symbols['_test_' + name]
        self.m.set_memory_block(0x100, b'\xfb\x00\xc3' + target.to_bytes(2, 'little'))
        self.m.pc = 0x100
        self.m.ticks_to_stop = 10_000_000
        self.port.writes.clear()
        self.port.reads.clear()
        api = {'read': 'read_port', 'mouse': 'read_mouse', 'sensitivity': 'get_mouse_sensitivity',
               'set': 'set_mouse_sensitivity'}.get(name, name)
        if (name == 'pad' or name == 'dispatch' and self.get('test_kind') == 0) and '_zgdk_controller_is_pad' in self.image.symbols:
            entry = self.image.symbols['_zgdk_controller_is_pad']
        elif name in ('pad', 'dispatch'):
            entry = self.image.symbols['_controller_is']
        else:
            entry = self.image.symbols['_controller_' + api]
        self.m.set_breakpoint(entry)
        self.m.run()
        assert self.m.pc == entry
        saved = self.m.ix, self.m.iy, self.m.alt_af, self.m.alt_bc, self.m.alt_de, self.m.alt_hl
        self.m.clear_breakpoint(entry)
        for _ in range(1000):
            self.m.run()
            if self.m.pc == 0:
                break
        assert self.m.pc == 0, (name, 'did not return', self.m.pc)
        assert self.m.sp == 0xf002
        assert self.m.iff1 and self.m.iff2
        if self.preserve:
            assert (self.m.ix, self.m.iy, self.m.alt_af, self.m.alt_bc, self.m.alt_de, self.m.alt_hl) == saved
        return 10_000_000 - self.m.ticks_to_stop


def axis(raw):
    return (-(raw & 127) if raw & 128 else raw) & 255


def movement(y, x):
    # Status LSB-first; motion bytes MSB-first on the wire.
    bits = 0x8000
    for i, raw in enumerate((y, x)):
        for b in range(8):
            bits |= ((raw >> (7 - b)) & 1) << (16 + i * 8 + b)
    return bits


def check(image, preserve=True):
    rng = random.Random(0x534e4553)
    pad_cycles = []
    pairs = [(0, 0), (65535, 65535)] + [(1 << b, 1 << (15 - b)) for b in range(16)]
    pairs += [(rng.randrange(65536), rng.randrange(65536)) for _ in range(200)]
    for p1, p2 in pairs:
        for port in (0, 1, 2, 255):
            run = Run(image, Port((p1, p2)), preserve)
            pad_cycles.append(run.call('read', port=port))
            assert run.get('test_word', 2) == (p1, p2, 0, 0)[(0, 1, 2, 255).index(port)]
            assert (run.get('PORT1_bits', 2), run.get('PORT2_bits', 2)) == (p1, p2)
            assert len(run.port.reads) == (16 if preserve else 32)
            assert run.port.index == 15
            if preserve:
                run.port.timing()
            run.call('get', port=port)
            assert run.get('test_word', 2) == (p1 if port == 0 else p2 if port == 1 else 0)
            assert not run.port.writes and not run.port.reads
    run = Run(image, preserve=preserve)
    run.put('PORT1_bits', 0xffff, 2)
    run.put('PORT2_bits', 0xffff, 2)
    run.put('MOUSE_x', 255)
    run.put('MOUSE_y', 127)
    run.call('flush')
    assert [run.get(n, s) for n, s in [('PORT1_bits', 2), ('PORT2_bits', 2), ('MOUSE_x', 1), ('MOUSE_y', 1), ('test_byte', 1)]] == [0] * 5
    run.call('init')
    assert [(a, v) for _, a, v in run.port.writes] == [(0xd2, 0xcf), (0xd2, 3), (0xd2, 3), (0xd0, 8)]
    mouse_cycles = []
    for port in (0, 1):
        for raw in range(256):
            for y, x in ((raw, 0), (0, raw), (raw, 255 - raw)):
                reports = [0x1234, 0xabcd]
                reports[port] = movement(y, x)
                run = Run(image, Port(reports), preserve)
                run.call('read', port=port)
                caches = run.get('PORT1_bits', 2), run.get('PORT2_bits', 2)
                mouse_cycles.append(run.call('mouse', port=port))
                assert run.get('test_y') == axis(y), (port, y, x)
                assert run.get('test_x') == axis(x), (port, y, x)
                assert run.get('test_byte') == bool(axis(y)) | (bool(axis(x)) << 1)
                assert (run.get('PORT1_bits', 2), run.get('PORT2_bits', 2)) == caches
                assert run.port.index == 31 and len(run.port.reads) == 16
                if preserve:
                    run.port.timing()
                    assert run.port.writes[0][0] >= 1884
    for port in (0, 1, 2, 255):
        for kind in (0, 1, 2, 255):
            for status in (0, 0x8000, 0xf000, 0x9000):
                run = Run(image, Port((status, status)), preserve)
                run.call('is', port=port, kind=kind)
                selected = status if port < 2 else 0
                expected = (selected & 0xf000) == (0 if kind == 0 else 0x8000) if kind < 2 else False
                assert run.get('test_byte') == expected
                assert run.port.index == (47 if kind == 1 else 15)
                if preserve:
                    run.port.timing()
    for port in (0, 1):
        run = Run(image, Port((0, 0)), preserve)
        run.call('pad', port=port)
        assert run.get('test_port') == port + 1 and run.get('test_byte') == 1
        for kind in (0, 1, 2, 255):
            run = Run(image, Port((0x8000, 0x8000)), preserve)
            run.call('dispatch', port=port, kind=kind)
            assert run.get('test_byte') == (kind == 1)
    # Detection consumes both motion and the clone-specific trailing report,
    # and must leave the decoded trailing coordinates visible to old callers.
    for tail_y, tail_x in ((255, 255), (128, 0), (1, 254)):
        tail = movement(tail_y, tail_x) >> 16
        for port in (0, 1):
            reports = [0, 0]
            reports[port] = movement(5, 133) | (tail << 32)
            run = Run(image, Port(reports), preserve)
            run.call('is', port=port, kind=1)
            assert run.get('test_byte') == 1
            assert (run.get('MOUSE_y'), run.get('MOUSE_x')) == (axis(tail_y), axis(tail_x))
            assert run.get('PORT1_bits' if port == 0 else 'PORT2_bits', 2) == 0x8000
            if preserve:
                run.port.timing()
    for port in (0, 1):
        for speed in range(3):
            for target in range(3):
                for clone in (False, True):
                    reports = [0, 0]
                    reports[port] = 0x8000
                    run = Run(image, Port(reports, port, speed, clone), preserve)
                    run.put('PORT1_bits', 0x1234, 2)
                    run.put('MOUSE_x', 77)
                    run.call('sensitivity', port=port)
                    assert run.get('test_byte') == speed
                    run.call('set', port=port, target=target)
                    succeeds = not clone or target == speed
                    assert run.get('test_byte') == (target if succeeds else 255)
                    assert run.port.cycles == ((target - speed - 1) % 3 + 1 if not clone else 1 if succeeds else 3)
                    assert run.get('PORT1_bits', 2) == 0x1234 and run.get('MOUSE_x') == 77
                    if preserve:
                        run.port.timing()
                    before = run.port.cycles
                    run.call('set', port=port, target=target)
                    assert run.port.cycles - before == (0 if succeeds else 3)
    # Successful initialization is per port and is deliberately not reset by
    # init/flush, matching the old static variable's lifetime.
    run = Run(image, Port((0x8000, 0x8000), mouse_port=0), preserve)
    run.call('set', port=0, target=0)
    assert run.port.cycles == 3
    run.call('flush')
    run.call('init')
    run.call('set', port=0, target=0)
    assert run.port.cycles == 3
    run.port.mouse_port = 1
    run.call('set', port=1, target=0)
    assert run.port.cycles == 6
    for port, target in ((2, 0), (255, 1), (0, 3), (1, 255)):
        run = Run(image, preserve=preserve)
        run.call('set', port=port, target=target)
        assert run.get('test_byte') == 255 and not run.port.writes
    for port in (0, 1, 2, 255):
        for status in (0, 0x8c00, 0xffff):
            run = Run(image, Port((status, status)), preserve)
            run.call('sensitivity', port=port)
            assert run.get('test_byte') == 255
            run.call('set', port=port, target=0)
            assert run.get('test_byte') == 255
            assert run.port.cycles == (3 if port < 2 else 0)
    return max(pad_cycles), max(mouse_cycles)


def main():
    directory = Path(sys.argv[1] if len(sys.argv) > 1 else 'build/controller-tests')
    image = Image(directory / 'api')
    times = check(image)
    pad, mouse = Image(directory / 'pad'), Image(directory / 'mouse')
    assert '_controller_read_mouse' not in pad.symbols
    assert '_controller_get_mouse_sensitivity' not in pad.symbols
    assert '_controller_set_mouse_sensitivity' not in pad.symbols
    assert '_controller_read_mouse' in mouse.symbols
    print(f'PASS: C ABI, both pads, all 256 axis encodings, detection, original/clone sensitivity, timing, archive separation')
    print(f'Assembly linked TEXT: pad {pad.text_size} B, mouse {mouse.text_size} B; max C-call read: pad {times[0]} T, mouse {times[1]} T')
    if len(sys.argv) > 2:
        baseline = Path(sys.argv[2])
        old_times = check(Image(baseline / 'api'), preserve=False)
        old_pad, old_mouse = Image(baseline / 'pad'), Image(baseline / 'mouse')
        print(f'Original C linked TEXT: pad {old_pad.text_size} B, mouse {old_mouse.text_size} B; max C-call read: pad {old_times[0]} T, mouse {old_times[1]} T')
        assert pad.text_size < old_pad.text_size and mouse.text_size < old_mouse.text_size
        assert times[0] < old_times[0] and times[1] < old_times[1]


if __name__ == '__main__':
    main()

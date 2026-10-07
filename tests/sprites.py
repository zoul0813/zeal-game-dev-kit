"""Execute real SDCC C callers with MMU-mapped VRAM and descriptor DMA.

Dependency: z80==1.2.0, shared with the controller tests. DMA models the
hardware's synchronous BUSREQ transfer; timing includes default 0x56 cycles.
"""
import sys
from pathlib import Path
import z80
from controller import Image


class Machine:
    def __init__(self, image, major=1, bank=8, dma=None):
        self.image = image
        self.m = z80.Z80Machine()
        self.pages = [0, 5, 11, 17]  # deliberately non-contiguous RAM mappings
        self.phys = bytearray(0x400000)
        self.major, self.bank, self.peripheral = major, bank, 3
        self.dma = (major in (1, 2)) if dma is None else dma
        self.probes = 0
        self.attempts = 0
        self.version_reads = 0
        self.descriptor = 0
        self.transfers = []
        self.writes = []
        self.dma_ticks = 0
        self.m.set_memory_block(0, image.memory)
        self.sync()
        self.m.set_input_callback(self.input)
        self.m.set_output_callback(self.output)
        self.m.set_breakpoint(0)

    def sync(self):
        for page, physical in enumerate(self.pages):
            start = page * 0x4000
            self.phys[physical * 0x4000:(physical + 1) * 0x4000] = self.m.memory[start:start + 0x4000]

    def refresh(self):
        for page, physical in enumerate(self.pages):
            self.m.set_memory_block(page * 0x4000, self.phys[physical * 0x4000:(physical + 1) * 0x4000])

    def input(self, port):
        low = port & 255
        if low == 0xf0:
            return self.pages[port >> 14]
        if low == 0x82:
            self.version_reads += 1
            return self.major
        if low == 0x8f:
            return self.bank
        if low == 0x8e:
            return self.peripheral
        raise AssertionError(('unexpected input', hex(port)))

    def output(self, port, value):
        low = port & 255
        self.writes.append((low, value))
        if low == 0xf0:
            self.sync()
            self.pages[low & 3] = value
            self.refresh()
        elif low == 0x8e:
            self.peripheral = value
        elif 0xa1 <= low <= 0xa3:
            assert self.peripheral == 4
            shift = 8 * (low - 0xa1)
            self.descriptor = (self.descriptor & ~(255 << shift)) | (value << shift)
        elif low == 0xa0:
            assert self.peripheral == 4 and value == 0x80
            self.attempts += 1
            if not self.dma:
                return
            self.sync()
            desc = self.phys[self.descriptor:self.descriptor + 12]
            source = int.from_bytes(desc[:3], 'little')
            dest = int.from_bytes(desc[3:6], 'little')
            size = int.from_bytes(desc[6:8], 'little')
            flags = desc[8]
            assert size and flags in (1, 5) and desc[9:] == b'\0\0\0'
            assert (self.descriptor & 0x3fff) + 12 <= 0x4000
            if flags == 1:
                assert (source & 0x3fff) + size <= 0x4000
            if self.bank * 0x20000 + 0x2800 <= dest < self.bank * 0x20000 + 0x2c00:
                self.transfers.append((source, dest, size, flags))
                assert dest + size <= self.bank * 0x20000 + 0x2c00
            else:
                assert size == 1 and flags == 1  # private RAM capability probe
                self.probes += 1
            for i in range(size):
                self.phys[dest + i] = self.phys[source + (i if flags == 1 else 0)]
            self.dma_ticks += (9 * 7 + size * 13) / 5  # 50MHz DMA, 10MHz CPU
            self.refresh()
        else:
            raise AssertionError(('unexpected output', hex(port), value))

    def addr(self, name):
        return self.image.symbols['_' + name]

    def put(self, name, value, size=1):
        self.m.set_memory_block(self.addr(name), value.to_bytes(size, 'little'))

    def get(self, name, size=1):
        addr = self.addr(name)
        return int.from_bytes(self.m.memory[addr:addr + size], 'little')

    def call(self, name, enabled=True, **args):
        for key, value in args.items():
            self.put('test_' + key, value, 1 if key == 'size' else 2)
        self.m.sp = 0xf000
        self.m.set_memory_block(0xf000, b'\0\0')
        self.m.ix, self.m.iy = 0x1234, 0x5678
        # Enter through page-one game code, never the page-zero VRAM window.
        target = self.addr('test_' + name)
        self.m.set_memory_block(0x7f00, bytes((0xfb if enabled else 0xf3, 0, 0xc3)) + target.to_bytes(2, 'little'))
        self.m.pc = 0x7f00
        self.m.ticks_to_stop = 10_000_000
        before = self.dma_ticks
        self.writes.clear()
        for _ in range(1000):
            self.m.run()
            if self.m.pc == 0:
                break
        assert self.m.pc == 0 and self.m.sp == 0xf002, (name, hex(self.m.pc), hex(self.m.sp))
        assert self.m.ix == 0x1234 and self.m.iy == 0x5678
        assert self.m.iff1 == enabled and self.m.iff2 == enabled
        assert self.pages == [0, 5, 11, 17] and self.peripheral == 3
        return 10_000_000 - self.m.ticks_to_stop + self.dma_ticks - before

    def hardware(self):
        self.sync()
        start = self.bank * 0x20000 + 0x2800
        return self.phys[start:start + 1024]


SPRITE = bytes((32, 0, 48, 0, 3, 1, 2, 0))


def init(m, arena=0xa000, size=128, enabled=True):
    m.call('init', arena=arena, size=size, enabled=enabled)
    assert m.get('test_error') == 0
    m.m.set_memory_block(m.addr('test_sprite'), SPRITE)


def register(m, n, arena=0xa000):
    for i in range(n):
        m.call('register')
        assert m.get('test_result', 2) == arena + 8 * i


def check(image):
    for major in (0, 1, 2, 255):
        for enabled in (False, True):
            m = Machine(image, major, bank=12)
            m.call('register'); assert m.get('test_result', 2) == 0
            m.call('clear')
            m.call('render', ctx=0); assert m.get('test_error') == 2
            for arena, size in ((0, 1), (0xa000, 0), (0xa000, 129)):
                m.call('init', arena=arena, size=size)
                assert m.get('test_error') == 2 and m.version_reads == 0
            init(m, enabled=enabled)
            assert m.version_reads == 0
            register(m, 128)
            m.call('register'); assert m.get('test_result', 2) == 0
            m.call('render', ctx=0x8000, enabled=enabled)
            assert m.get('test_error') == 0 and m.hardware() == SPRITE * 128
            assert bool(m.transfers) == (major in (1, 2))
            # Hardware capability is cached, even if the version port changes.
            m.major = 255  # render must not consult the firmware version
            m.call('clear')
            assert bytes(m.m.memory[0xa000:0xa400]) == bytes(1024)
            register(m, 1)
            m.call('render', enabled=enabled)
            assert m.hardware() == SPRITE + bytes(1016)
            init(m, size=1)
            register(m, 1)
            m.call('register'); assert m.get('test_result', 2) == 0
            m.call('clear')
            m.call('render', enabled=enabled)
            assert m.hardware() == bytes(1024)
            assert m.version_reads == 0 and m.attempts == (len(m.transfers) + 2 if m.dma else 2)
            m.call('render', ctx=0); assert m.get('test_error') == 2
    # Non-DMA small uploads on DMA hardware, and source MMU splits in DMA.
    for n in (0, 1, 7, 8, 127, 128):
        m = Machine(image)
        init(m, arena=0xbff9)
        register(m, n, arena=0xbff9)
        m.call('render', ctx=0x8000)
        assert m.hardware()[:n * 8] == SPRITE * n
        assert bool(m.transfers) == (n >= 8)
        if n >= 8:
            assert [t[2] for t in m.transfers] == [7, n * 8 - 7]
        assert m.version_reads == 0
    # Page-zero ROM arenas must survive fallback VRAM mapping; DMA also works.
    for major in (0, 1):
        m = Machine(image, major)
        init(m, arena=0x3ffc)
        register(m, 16, arena=0x3ffc)
        m.call('render', ctx=0x8000)
        assert m.hardware()[:128] == SPRITE * 16
    # Capability comes from the functional probe, independently of version.
    for major, supported in ((0, True), (1, False), (255, True)):
        m = Machine(image, major, dma=supported)
        init(m); register(m, 8)
        m.call('render', ctx=0x8000)
        assert bool(m.transfers) == supported and m.hardware()[:64] == SPRITE * 8
        assert m.attempts == (2 if supported else 1) and m.version_reads == 0
    # Direct C callers verify x/y order and the stack argument cleanup.
    m = Machine(image)
    m.m.set_memory_block(m.addr('test_sprite'), SPRITE)
    m.call('get_point')
    assert bytes(m.m.memory[m.addr('test_point'):m.addr('test_point') + 4]) == bytes((48, 0, 32, 0))
    m.m.set_memory_block(m.addr('test_point'), bytes((0x34, 0x12, 0x78, 0x56)))
    m.call('set_point')
    assert bytes(m.m.memory[m.addr('test_sprite'):m.addr('test_sprite') + 8]) == bytes((0x78, 0x56, 0x34, 0x12, 3, 1, 2, 0))
    m.call('set_xy')
    assert bytes(m.m.memory[m.addr('test_sprite'):m.addr('test_sprite') + 4]) == bytes((0x78, 0x56, 0x34, 0x12))
    timings = []
    for major in (0, 1):
        m = Machine(image, major)
        init(m); register(m, 128)
        timings.append(m.call('render', ctx=0x8000))
    print('PASS: C ABI, capacity/reuse, stale slots, cached DMA, LDIR, relocated ZVB, MMU splits, IFF preservation, x/y helpers')
    print(f'128-sprite C-call upload: LDIR {timings[0]:.0f} T, DMA {timings[1]:.0f} T (DMA default clocks modeled)')
    print(f'Linked assembly TEXT (including callers): {image.text_size} bytes')


def check_descriptor_boundary(image):
    m = Machine(image)
    init(m); register(m, 128)
    m.call('render', ctx=0x8000)
    assert m.hardware() == SPRITE * 128
    # buffer is at 0xbffb; the selected descriptor is at 0xc007.
    assert m.descriptor & 0x3fff == 7
    m.call('clear'); m.call('render')
    assert m.hardware() == bytes(1024) and m.probes == 1
    print('PASS: descriptor buffer crossing discontiguous MMU pages')


if __name__ == '__main__':
    check(Image(Path(sys.argv[1]) / 'api'))
    check_descriptor_boundary(Image(Path(sys.argv[1]) / 'boundary'))

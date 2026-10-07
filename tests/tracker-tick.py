"""Check real SDCC tick scheduling and optionally compare a compiled baseline.

Build with tests/build-tracker-tick.sh. Dependency: z80==1.2.0, as used by
controller and sprite tests. Times cover only zmt_tick(), excluding setup.
"""
import sys
from pathlib import Path
import z80
from controller import Image


class Tick:
    def __init__(self, image):
        self.image = image
        self.m = z80.Z80Machine()
        self.m.set_memory_block(0, image.memory)
        self.m.set_input_callback(lambda port: 3)
        self.m.set_output_callback(lambda port, value: None)

    def put(self, name, value):
        self.m.memory[self.image.symbols['_' + name]] = value

    def get(self, name):
        return self.m.memory[self.image.symbols['_' + name]]

    def until(self, pc):
        self.m.set_breakpoint(pc)
        for _ in range(1000):
            self.m.run()
            if self.m.pc == pc:
                break
        assert self.m.pc == pc, ('did not reach breakpoint', hex(pc), hex(self.m.pc))
        self.m.clear_breakpoint(pc)

    def call(self, tempo, phase):
        self.put('test_tempo', tempo)
        self.put('test_phase', phase)
        self.m.sp = 0xf000
        self.m.set_memory_block(0xf000, b'\0\0')
        target = self.image.symbols['_test_tick']
        self.m.set_memory_block(0x7f00, b'\xfb\x00\xc3' + target.to_bytes(2, 'little'))
        self.m.pc = 0x7f00
        self.m.ticks_to_stop = 10_000_000
        self.until(self.image.symbols['_zmt_tick'])
        return_pc = int.from_bytes(self.m.memory[self.m.sp:self.m.sp+2], 'little')
        start = self.m.ticks_to_stop
        self.until(return_pc)
        cycles = start - self.m.ticks_to_stop
        frames = tempo >> 2
        normalized = 0 if frames and phase >= frames * 4 else phase
        boundary = bool(frames and normalized % frames == 0)
        assert self.get('zmt_ticks') == (normalized + 1 if frames else phase)
        assert self.get('zmt_next_step') == 5 + boundary
        return cycles


def check(directory):
    image = Image(Path(directory) / 'api')
    tick = Tick(image)
    # Exercise all phases at short, fractional, normal and maximal tempos.
    for tempo in (0, 1, 2, 3, 4, 7, 8, 9, 31, 32, 33, 128, 252, 255):
        for phase in range(256):
            tick.call(tempo, phase)
    return image, [tick.call(32, phase) for phase in range(32)]


if __name__ == '__main__':
    image, current = check(sys.argv[1])
    assert not any('moduchar' in name for name in image.symbols)
    print('PASS: SDCC tick cadence, quarter wrap, paused tempos; no remainder helper linked')
    if len(sys.argv) > 2:
        before, original = check(sys.argv[2])
        assert any('moduchar' in name for name in before.symbols)
        assert all(new < old for new, old in zip(current, original))
        savings = sum(old-new for old, new in zip(original, current)) / len(current)
        print(f'Tempo 32, 10MHz: mean zmt_tick savings {savings:.1f} T = {savings/10000:.4f} ms/frame')
        print(f'Idle frames: {min(original[1:8])}–{max(original[1:8])} T before, '
              f'{min(current[1:8])}–{max(current[1:8])} T after')
        old_size = before.text_size + before.symbols['l__CODE']
        new_size = image.text_size + image.symbols['l__CODE']
        print(f'Linked code (TEXT + CODE): {old_size} -> {new_size} bytes')

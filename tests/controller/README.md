# Controller assembly checks

`reference/` preserves the controller implementation from commit `f7253fa`.
Only its controller-header include was redirected to the frozen `reference.h`;
`pio.h` and the timing-sensitive C are otherwise unchanged. These fixtures are
compiled as test references, never into the library. Keep them unchanged when
refactoring production code: they establish the pre-rewrite electrical baseline.

Run from the repository root inside the Zeal development environment with
`ZOS_PATH`, `ZVB_SDK_PATH`, and `COREUTILS_PATH` set:

```sh
sh tests/controller/build.sh
cmake -S . -B build-controller/library
cmake --build build-controller/library
python3 tests/controller/check_link.py
```

Then run on the host:

```sh
ZEAL_EMULATOR_PATH=/path/to/Zeal-NativeEmulator sh tests/run-controller.sh
sh tests/run-input.sh
```

The emulator path defaults to the sibling `../Zeal-NativeEmulator` checkout.
The test links its existing `hw/z80.c`, `hw/mmu.c`, and `utils/vtimer.c`, without
modifying or building its application. The checkout must provide the current
`z80_step`, MMU mapping, and device APIs. This is a test-only source dependency;
games do not depend on the emulator. `CC` can select the host C compiler.
`CONTROLLER_BUILD_DIR` selects an alternative fixture build directory; pass the
same directory as the first argument of `check_link.py`.

The harness executes real instructions with ASan/UBSan, scripted active-low PIO
input, and timestamped latch/clock/read traces. It checks:

- Single-port reads across 3,856 raw patterns per port, plus unrelated data-line
  activity; simultaneous dual-port reads and every serialized button position.
- All 256 raw values of each mouse axis on each port, plus a zero/sign/extreme
  matrix, buttons, invalid signatures, movement flags, and 48-bit detection.
- Sensitivity getter codes, every initial/target combination, per-port first-use
  state, three-cycle limits, clones ignoring software cycling, and invalid input.
- Each port's original minimum phase/settling intervals, start and byte gaps,
  detection continuation, and report/cycle recovery.
- Stack balance, IX/IY and alternate-register preservation, enabled/disabled
  interrupt state, and calls emitted by SDCC for the public C interface.

`check_link.py` compiles and links twelve minimal C applications against the
actual archive, then verifies exact capability symbols, absence of controller
snapshot globals, and single definitions of shared helpers. The sensitivity
getter must link no persistent state; the setter must link exactly one byte.
It also links the old archive for comparable footprint measurements.

To verify the Makefile route separately:

```sh
make BUILD_DIR=build-controller/make LIB_DIR=build-controller/make-lib
python3 tests/controller/check_link.py build-controller build-controller/make-lib/zgdk.lib
```

Both builds need SDCC/SDASZ80; the Makefile now uses the OS `sdk/sdcc/include`
location and the coreutils include dependency already required by CMake. It
recreates the archive so obsolete C members cannot survive the ASM migration.

## Measurements

Measured with SDCC 4.4.0 and the native Z80 core, without interrupts or external
wait states. Linked byte counts include initialization and the small C test
caller, but not an OS startup image. Output values are deliberately consumed in
the runtime harness; these are not empty-call timings.

| Minimal application | Previous C | Assembly |
| --- | ---: | ---: |
| Initialization only | 436 code bytes | 20 code bytes |
| Single pad, port 2 | 441 code bytes | 114 code bytes |
| Full mouse, port 2 | 446 code bytes | 258 code bytes |

Single-pad polling has no persistent report storage. Only games calling the
explicit two-pad API supply a four-byte `ControllerPads`; a one-pad result is
returned in the normal 16-bit C return registers.

| Routine body benchmark | Previous C | Assembly |
| --- | ---: | ---: |
| Pad, port 2, both lines `0xFFFF` | 6,160 T / 598 instructions | 6,033 T / 565 instructions |
| Mouse, port 2, header `0x8300`, raw Y `0xFF`, X `0xAA` | 22,726 T / 2,871 instructions | 22,717 T / 2,009 instructions |

The previous full mouse benchmark combines its header and continuation routines.
Counts include their return instructions, without external C call-site overhead.
The assembly mouse routine completes the whole transaction in one call.
Runtime improvements are intentionally modest: preserving the old electrical
intervals retains nearly all bus time, even where fewer instructions suffice.

Selected interval minima, in T-states:

| Interval | Previous minimum | Assembly minimum |
| --- | ---: | ---: |
| Normal latch high / clock low | 18 / 18 | 18 / 18 |
| Repeated header clock high | 367 | 369 |
| Mouse clock rise to sample | 482 | 482 |
| Mouse repeated clock high | 806 | 809 |
| Mouse Y-to-X gap, port 2 | 1,223 | 1,226 |

The full test compares additional intervals separately for each port. At 10 MHz,
10 T-states equal one microsecond. These are CPU-visible lower bounds with the
existing interrupt policy, not a promise of uninterrupted wall-clock timing.

Physical SNS-016 and Hyperkin qualification has not been performed. Before
hardware release, exercise both ports, full-range movement, both mouse buttons,
software sensitivity on SNS-016, fixed-speed clone behavior, and detection with
original and clone devices. Automated tests prove waveform timing and decoding;
they do not substitute for physical device qualification.

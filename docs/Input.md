# ZGDK: Input

ZGDK supports both Keyboard and SNES controller input. By default, ZGDK maps the users keyboard
to look like a SNES controller to help simplify supporting both devices.

> [!NOTE]
> If you game has advanced keyboard features, you may want to implement your own keyboard logic.

You can initialize ZGDK input with

```C
/**
 * @brief Initialize input
 *
 * @param use_controller Attempt to use a controller?
 */
zos_err_t input_init(uint8_t use_controller)
```

Example

```C
zos_err_t err = input_init(true);
```

To read user input, call `input_read()`. The old `input_get()` alias remains
available automatically and emits a deprecation warning when used.

```C
/**
 * @brief Get user input
 *
 * @return 16-bit mask of ZGDK Inputs
 */
uint16_t input_read(void);
```

Example

```C
uint16_t x, y;
while (1) {
  uint16_t input1 = input_read();

  if((input1 & BUTTON_LEFT)) x--;
  if((input1 & BUTTON_RIGHT)) x++;
  if((input1 & BUTTON_UP)) y--;
  if((input1 & BUTTON_DOWN)) y++;

  // Quit if Select is pressed
  if((input1 & BUTTON_SELECT)) return 0;
}
```

You may find the following macros handy

```C
#define LEFT1   (input1 & BUTTON_LEFT)
#define RIGHT1  (input1 & BUTTON_RIGHT)
#define UP1     (input1 & BUTTON_UP)
#define DOWN1   (input1 & BUTTON_DOWN)
#define BUTTON1_B  (input1 & BUTTON_B)
#define BUTTON1_A  (input1 & BUTTON_A)
#define START1 (input1 & BUTTON_START)
#define SELECT1 (input1 & BUTTON_SELECT)
```

Which can then be used as follows, to simplify your code

```C
if(LEFT1) x--;
if(RIGHT1) x++;
```

### Held buttons and flushing

`input_read()` combines keyboard buttons with a valid SNES pad report from port 1.
Mice and invalid controller reports are ignored.
After initialization or an invalid report, pad buttons must be seen released
before they can activate.

Opposing keyboard directions prefer the most recently pressed key. Releasing it
restores the other direction if still held.

`input_flush()` drains queued keyboard events and clears keyboard state. It also
suppresses currently held pad buttons until release, preventing an immediate
retrigger. This suppression applies to `input_read()`, not raw controller reads.

### Raw controller polling

Initialize the PIO once with `controller_init()`. Reports belong to the caller;
there are no controller snapshot globals or heap allocations.

A single pad needs only its returned 16-bit value. It does not allocate a
`ControllerPads` structure or link the dual-port reader:

```c
uint16_t buttons = controller_read_pad(SNES_PORT1);
if (buttons & BUTTON_A) jump();
```

Reports retain all 16 raw bits; `(buttons & 0xF000) == 0` identifies a valid pad report.
An invalid port returns `0xFFFF` without accessing the PIO.

For two pads, use a single latch/clock transaction:

```c
ControllerPads pads;  /* Four bytes, only when the game requests two reports. */
controller_read_pads(&pads);
```

Mouse polling reads the entire 32-bit report in one call:

```c
ControllerMouse mouse;  /* Raw 16-bit header, signed X and Y: four bytes. */
uint8_t moved = controller_read_mouse(SNES_PORT2, &mouse);
if (moved != MOUSE_ERROR) {
    x += mouse.x;
    y += mouse.y;
    if (mouse.bits & MOUSE_LEFT) click();
}
```

The return value has Y movement in bit 0 and X movement in bit 1. Axes range
from -127 through 127; both encodings of zero become zero. A bad signature
clears the report and returns `MOUSE_ERROR` (`0xFF`). Invalid ports and NULL
outputs perform no I/O; NULL dual-pad outputs also perform no I/O.

`controller_is_pad(port)` and `controller_is_mouse(port)` are separate detection
calls. Mouse detection additionally consumes the legacy trailing 16 bits.
Detection consumes input, so polling an already configured device directly is
cheaper than detecting it every frame.

Single-port calls sample only the selected DATA line. CLOCK and LATCH are
physically shared, so they still pulse both connected devices. Poll two pads
with `controller_read_pads()` when their snapshots must share one latch.
Pad-only, mouse-only, and dual-port readers are separate archive members;
shared transport and timing helpers are linked once. Sensitivity support is
linked only when requested, and only its setter owns persistent state (one byte).

### Deprecated aliases

Existing aliases remain available through the normal namespace headers; games
need no extra includes to keep calling them:

```c
#include <zgdk/input.h>

uint16_t input = input_get();       /* Deprecated: use input_read(). */
uint16_t pad = controller_read();   /* Deprecated: use controller_read_pad(SNES_PORT1). */
```

Each namespace keeps its deprecated declarations separate and includes them
automatically after its canonical declarations:

| Namespace header | Included deprecated header | Alias |
| --- | --- | --- |
| `zgdk/input.h` | `zgdk/input/deprecated.h` | `input_get()` |
| `zgdk/input/controller.h` | `zgdk/input/controller/deprecated.h` | `controller_read()` |

The umbrella `zgdk.h` also exposes these aliases through its normal includes.
Including a namespace header alone emits no warning; each use of a deprecated
alias emits a compiler warning naming its replacement. Aliases remain macros
and add no wrapper code or persistent state.

SDCC 4.4's bundled **sdcpp** preprocessor supports the `GCC warning` pragma used
for these use-site diagnostics. Despite the directive's name, it does not
require compiling with GCC or using unsupported GCC function attributes. The
shared warning helper is in `zgdk/deprecation.h`; GCC and Clang host checks
also verify its behavior.

This organization does **not** restore the old controller API. In particular,
`controller_read()` reads only port 1 and does not update global snapshots or
poll port 2. The removed getters, detection API, and split mouse API still need
the migrations below.

### Migrating low-level controller callers

The controller API intentionally breaks compatibility with the old global-state
interface. Recompile games against the matching header and library.

| Old usage | Replacement |
| --- | --- |
| `controller_read_port(port)` | `controller_read_pad(port)`, sampling only that port |
| A read implicitly refreshing both ports | `controller_read_pads(&pads)` |
| `PORT1_bits`, `PORT2_bits`, getter/pressed macros | Store the returned report; test its button bits |
| Header read followed by `controller_read_mouse(port)` | `controller_read_mouse(port, &mouse)` reads the whole report |
| `MOUSE_x`, `MOUSE_y`, axis getters | Caller-owned `mouse.x`, `mouse.y` |
| `controller_is(port, type)` and `ControllerType` | Separate `controller_is_pad(port)` / `controller_is_mouse(port)` |
| `controller_flush()` | Clear the game's own saved reports |

`input_init()`, `input_read()`, and `input_flush()` retain their public interfaces
and held-button suppression behavior. Their controller path now links only
single-port pad polling. Sensitivity function signatures and button masks are
unchanged.

### Controller timing and verification

Pad polling follows [Jim Christy's measured SNES waveform](https://www.lasersam.org/REPAIR/F_SNES.html#5):
a latch of at least 12 µs and clock phases/settling of at least 6 µs. These are
10 MHz CPU timing targets; loop overhead can make a phase longer. Pad reads
no longer carry the old C implementation's extra per-bit padding.

Mouse polling keeps its longer legacy waits. It also enforces the
[SNESdev Hyperkin bounds](https://snes.nesdev.org/wiki/Mouse#Hyperkin_Mouse):
at least 170 SNES master clocks between bit reads and 336 between bits 16 and 17.
The tests conservatively round these to 8 µs and 16 µs (80 and 160 Z80 T-states
at 10 MHz). SNES master clocks are not Z80 or NES CPU cycles. Sensitivity
reports retain their separate timing profile. No polling routine changes the
CPU interrupt-enable state.

The instruction-level test executes linked Z80 code against scripted PIO input,
using the unmodified Zeal-NativeEmulator CPU. It checks pad protocol bounds and mouse timing separately for
each port, including mouse start/inter-byte/detection gaps and sensitivity
recovery. Separate native link tests verify exactly which archive members each
API pulls in. See [controller test instructions](../tests/controller/README.md)
for reproduction, measurements, and hardware qualification status.

### SNES Mouse Sensitivity

Original SNS-016 mice support software-selectable sensitivity:

```C
MouseSensitivity controller_get_mouse_sensitivity(uint8_t port);
uint8_t controller_set_mouse_sensitivity(uint8_t port, MouseSensitivity s);
```

`controller_set_mouse_sensitivity()` returns the achieved `MouseSensitivity`, or `0xFF` if the port, requested sensitivity, mouse signature, or convergence check fails.

Hyperkin-style clone mice may ignore software sensitivity cycling and always report low sensitivity. Use the physical button on those mice instead.

### Control Mapping

| SNES   | Keyboard |
| ------ | -------- |
| Up     | Up       |
| Down   | Down     |
| Left   | Left     |
| Right  | Right    |
| B      | Z/Space  |
| A      | X        |
| Y      | A        |
| X      | S        |
| Select | R Shift/Quote |
| Start  | Enter    |
| L      | Q        |
| R      | W        |

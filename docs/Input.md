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

To read user input, you can call `input_get()`

```C
/**
 * @brief Get user input
 *
 * @return 16-bit mask of ZGDK Inputs
 */
uint16_t input_get(void);
```

Example

```C
uint16_t x, y;
while (1) {
  uint16_t input1 = input_get();

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

### SNES Mouse Sensitivity

Original SNS-016 mice support software-selectable sensitivity:

```C
MouseSensitivity controller_get_mouse_sensitivity(uint8_t port);
uint8_t controller_set_mouse_sensitivity(uint8_t port, MouseSensitivity s);
```

`controller_set_mouse_sensitivity()` returns the achieved `MouseSensitivity`, or `0xFF` if the port, requested sensitivity, mouse signature, or convergence check fails.

Hyperkin-style clone mice may ignore software sensitivity cycling and always report low sensitivity. Use the physical button on those mice instead.

### SNES assembly and linking

The SNES modules are SDASZ80 `.asm` files using SDCC's default Z80 `__sdcccall(1)`
ABI (SDCC 4.2 or newer). Existing C functions, macros, button masks, globals and
return values are unchanged. Both Make and CMake assemble them into `zgdk.lib`.

`controller_read_port()` latches once and samples both ports, updates both
`PORT1_bits` and `PORT2_bits`, and returns the requested port. An invalid port
still refreshes both caches and returns zero. `controller_read_mouse()` continues
that same report without latching: read the status first, then the mouse on the
same port. Mouse axes remain sign/magnitude decoded to signed deltas, with the
same Y/X movement flags. Sensitivity reads do not update button or axis caches.

Pad reads, initialization, flushing and constant `controller_is(port, SNES_PAD)`
calls do not link mouse protocol code. Flush still clears the two mouse coordinate
bytes for compatibility. The header inlines constant pad detection into a separate
archive entry; arguments are evaluated once. Runtime detection and function
pointers retain the original `controller_is` symbol and its mouse support.
Mouse movement and sensitivity are separate archive members, so using movement
alone does not include sensitivity code. Generic mouse detection still consumes
both motion and trailing bytes and leaves the last decoded axes visible, as before.

Timing assumes a **10 MHz Z80** with no added I/O wait states:

| Operation | Minimum deliberate timing |
| --- | --- |
| Normal latch high | 124 T (12.4 us) |
| Latch release to first status sample | 66 T (6.6 us) |
| Clock low | 72 T (7.2 us) |
| Rising clock to status sample | 77 T (7.7 us) |
| Rising clock to mouse-axis sample | 482 T (48.2 us) |
| Mouse continuation startup delay | 1901 T (190.1 us) |

Mouse settling and startup delays retain the original implementation's extra
time for Nintendo and Hyperkin mice. Software sensitivity still forces a cycle
on the first successful set for each port, retries at most three times, and returns
`0xFF` if the requested setting cannot be confirmed. Init/flush do not reset that
per-port initialization state. Routines preserve IX/IY, alternate registers and
interrupt state; callers must serialize access to the shared PIO/report stream.

Measured with SDCC 4.4.0 `--opt-code-speed`, using the C callers in
`tests/controller_link.c` (including pad detection) and `tests/controller_api.c`:

| Probe | Original C | Assembly |
| --- | ---: | ---: |
| Pad-only linked `_TEXT` | 578 bytes | 191 bytes |
| Full mouse linked `_TEXT` | 1125 bytes | 517 bytes |
| Worst measured pad read via C caller | 6238 T | 3744 T |
| Worst measured mouse movement read via C caller | 16823 T | 12476 T |

These are complete probe code sizes and include their C callers. Actual game
savings depend on which archive members it uses. The bundled `lib/zgdk.lib` is
rebuilt with the assembly implementation.

To reproduce the functional, ABI, link separation and cycle-trace checks:

```sh
ZOS_PATH=/path/to/Zeal-8-bit-OS sh tests/build-controller.sh
python3 -m venv /tmp/zgdk-controller-tests
/tmp/zgdk-controller-tests/bin/pip install z80==1.2.0
/tmp/zgdk-controller-tests/bin/python tests/controller.py build/controller-tests
```

The tests execute the linked machine code, cover every mouse axis encoding on
both ports, and model original/clone sensitivity behavior and trailing reports.
They do not replace testing on physical Nintendo and Hyperkin hardware.

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
| Select | R Shift  |
| Start  | Enter    |
| L      | Q        |
| R      | W        |

# Controller API migration guide

The controller API now reads directly into values owned by your game. It no
longer maintains global pad or mouse snapshots. Pad polling, mouse polling,
detection, and sensitivity control are separate entry points, so games link
only the capabilities they use.

Start by identifying how your game reads input:

| Your game uses                                          | What to change                                                                   |
| ------------------------------------------------------- | -------------------------------------------------------------------------------- |
| `input_init()`, `input_read()`, and `input_flush()`     | No controller API changes required.                                              |
| `input_get()`                                           | Replace it with `input_read()` to remove its deprecation warning.                |
| Only the value returned by `controller_read()`          | No rename needed. It is a supported alias for `controller_read_pad(SNES_PORT1)`. |
| Controller globals, getters, or `controller_pressed*()` | Keep the result of a poll in your own game state.                                |
| One read to refresh both controller ports               | Use `controller_read_pads()`.                                                    |
| `controller_is(port, type)`                             | Use `controller_is_pad(port)` or `controller_is_mouse(port)`.                    |
| A header read followed by a mouse movement read         | Replace both with one complete `controller_read_mouse(port, &report)` call.      |

`controller_read()` remains a supported convenience alias. `input_get()` is
deprecated but remains available automatically through the normal input header,
including through `zgdk.h`. Do not add an explicit `deprecated.h` include. The
removed APIs listed below do not have compatibility wrappers.

Rebuild the SDK and recompile your game against its matching headers and
`lib/zgdk.lib`. Previously compiled game objects may still reference removed
symbols. Keep your existing choice of `zgdk.h`, `zgdk/input.h`, or
`zgdk/input/controller.h`; changing the include is not part of the migration.

## Replace the old calls

| Old API or state                                                           | New API or replacement                                                          |
| -------------------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| `controller_read_port(port)`                                               | `controller_read_pad(port)` for one pad; `controller_read_pads(&pads)` for both |
| `controller_read()`                                                        | Retained as a supported alias for `controller_read_pad(SNES_PORT1)`             |
| `controller_get(port)`                                                     | Read your saved report; it is no longer an SDK getter                           |
| `PORT1_bits`, `PORT2_bits`                                                 | A returned `uint16_t`, or `ControllerPads.port1` and `.port2`                   |
| `controller_get_port1()`, `controller_get_port2()`                         | Your saved pad values                                                           |
| `controller_pressed(button)` or `controller_pressed_port1(button)`         | `controller_pressed(pad1, button)`                                              |
| `controller_pressed_port2(button)`                                         | `controller_pressed(pad2, button)`                                              |
| `controller_is(port, SNES_PAD)`                                            | `controller_is_pad(port)`                                                       |
| `controller_is(port, SNES_MOUSE)`                                          | `controller_is_mouse(port)`                                                     |
| `ControllerType`, `SNES_PAD`, `SNES_MOUSE`                                 | Remove the shared type-based dispatch; use the separate detection calls         |
| `controller_read_mouse(port)`                                              | `controller_read_mouse(port, &mouse)`, which reads the entire report            |
| `MOUSE_x`, `MOUSE_y`, `controller_get_mousex()`, `controller_get_mousey()` | `ControllerMouse.x` and `.y`                                                    |
| `controller_get(mouse_port)` for mouse buttons                             | `ControllerMouse.bits & (MOUSE_LEFT \| MOUSE_RIGHT)`                            |
| `controller_flush()`                                                       | Clear or refresh your game's saved reports and edge state                       |

`SNES_PORT1` and `SNES_PORT2` still mean port indices **0 and 1**. Button masks,
mouse button masks, and `MouseSensitivity` values are unchanged.

The old `controller_pressed*()` macros tested whether a button was held. They
did not detect a new press. The supported `controller_pressed(buttons, mask)`
macro now takes your saved report explicitly and returns `buttons & mask`.
It performs no hardware read and evaluates each argument once. A combined mask,
such as `controller_pressed(buttons, BUTTON_A | BUTTON_Y)`, tests whether **any**
of those buttons are held. To require both, compare the result with the full
mask. To detect a new press, calculate `buttons & ~previous_buttons` instead.

The old one-argument form must be updated; it has no compatibility wrapper.

## Read one pad

The old read updated both global snapshots, even when its argument selected
only one return value. The new single-pad read samples only the selected DATA
line and returns its report. It does not refresh any other state.

**Before**

```c
#include <zgdk/input/controller.h>

void jump(void);  /* Your game's existing jump function. */

void poll_player_one(void)
{
    controller_read();
    if (controller_pressed(BUTTON_A)) {
        jump();
    }
}
```

**After**

```c
#include <zgdk/input/controller.h>

void jump(void);  /* Your game's existing jump function. */

void poll_player_one(void)
{
    uint16_t buttons = controller_read();  /* Same as controller_read_pad(SNES_PORT1). */
    if (controller_pressed(buttons, BUTTON_A)) {
        jump();
    }
}
```

Both versions call `jump()` when Button A is held. The change is where the
button state comes from: the old macro inspected an SDK global; the new code
tests the returned report. Holding A across several polls still calls `jump()`
on each poll, just as before.

Use the returned value for all button checks in that game tick. If several
systems need the same report, save it in your game state and pass it to them.
Calling a polling function again is another hardware read, not a getter.

A single pad requires only one 16-bit result. You do not need a `ControllerPads`
struct or storage for a second pad. The SDK adds no global snapshot storage.

### Detect at startup; poll during gameplay

Use `controller_is_pad(port)` during initialization to identify a pad, as shown
in [Detect devices at initialization](#detect-devices-at-initialization). For a
fixed controller setup, the normal gameplay loop then reads the pad and checks
its buttons. No additional detection call is required for each read.

`controller_is_pad(port)` performs its own hardware read. It does not validate
an already saved report, so calling it before `controller_read_pad(port)` would
read twice and check a different sample.

Games that support unplugging or replacing controllers during gameplay should
also validate each returned report before using its button bits. Raw pad reads
do not reject invalid signatures. Use `controller_pad_valid(buttons)` to check
the saved report without another hardware read:

```c
if (controller_pad_valid(buttons) && controller_pressed(buttons, BUTTON_A)) {
    jump();
}
```

This macro checks the standard pad signature and evaluates its argument once.
It does not track state or suppress held buttons. Raw polling callers own that
policy; `input_read()` uses the same check internally and also manages
held-button suppression. The mixed-device example below uses the macro too.

Invalid port arguments return `0xFFFF` without I/O; the old
`controller_read_port()` returned zero for an invalid argument.

`controller_read()` is supported and is not deprecated. It is shorthand for
`controller_read_pad(SNES_PORT1)`; use the longer form when selecting a port.
The alias is **not** fully backward compatible with games that relied on its
old updates to both global snapshots. Such games must migrate their saved state
as well.

## Read two pads

Use the dual-pad entry point when both players should be sampled from one
latch operation. It returns both reports through a four-byte caller-owned
struct.

**Before**

```c
#include <zgdk/input/controller.h>

void poll_players(uint16_t *player_one, uint16_t *player_two)
{
    controller_read_port(SNES_PORT1);  /* Also refreshed port 2. */
    *player_one = controller_get(SNES_PORT1);
    *player_two = controller_get(SNES_PORT2);
}
```

**After**

```c
#include <zgdk/input/controller.h>

void poll_players(uint16_t *player_one, uint16_t *player_two)
{
    ControllerPads pads;
    controller_read_pads(&pads);

    *player_one = pads.port1;
    *player_two = pads.port2;
}
```

Both versions return the raw reports through the same output pointers. For
games that support controller changes during play, validate each report before
using its button bits, as described for the single-pad example.
Pass valid output pointers to this helper.

Do not replace the old code with two getters: there are no SDK snapshots to
query. Two separate `controller_read_pad()` calls are also two separate latch
operations. Use `controller_read_pads()` when you need the shared snapshot.

## Detect devices at initialization

Replace the shared detection function with the device-specific call.

**Before**

```c
#include <zgdk/input/controller.h>

void detect_devices(uint8_t *has_pad, uint8_t *has_mouse)
{
    *has_pad = controller_is(SNES_PORT1, SNES_PAD);
    *has_mouse = controller_is(SNES_PORT2, SNES_MOUSE);
}
```

**After**

```c
#include <zgdk/input/controller.h>

void detect_devices(uint8_t *has_pad, uint8_t *has_mouse)
{
    *has_pad = controller_is_pad(SNES_PORT1);
    *has_mouse = controller_is_mouse(SNES_PORT2);
}
```

Both return a boolean, and an invalid port returns false without I/O. Detection
reads the device; it does not populate a snapshot for a later getter. Mouse
detection also consumes the extra trailing 16 bits used by the legacy detection
sequence. Normally detect at startup or during an explicit reconnect check,
then use the polling call during gameplay.

Initialize the hardware once with `controller_init()`, or keep using
`input_init(1)` if your game also uses ZGDK keyboard initialization. Do not call
`controller_init()` every frame or use it as a substitute for flushing game
state.

## Read a complete mouse report

The old mouse function continued an already started transaction. Games first
read the 16-bit header, then clocked the mouse's movement bytes and read global
axis values. The new function performs the complete 32-bit transaction itself.

**Before**

```c
#include <zgdk/input/controller.h>

uint16_t poll_mouse(int16_t *cursor_x, int16_t *cursor_y)
{
    controller_read_port(SNES_PORT1);  /* Read both headers. */
    uint16_t buttons = controller_get(SNES_PORT2);
    if (controller_read_mouse(SNES_PORT2)) {
        *cursor_x += controller_get_mousex();
        *cursor_y += controller_get_mousey();
    }
    return buttons & (MOUSE_LEFT | MOUSE_RIGHT);
}
```

**After**

```c
#include <zgdk/input/controller.h>

uint16_t poll_mouse(int16_t *cursor_x, int16_t *cursor_y)
{
    ControllerMouse mouse;
    uint8_t result = controller_read_mouse(SNES_PORT2, &mouse);

    if (result == MOUSE_ERROR) {
        return 0;
    }

    *cursor_x += mouse.x;
    *cursor_y += mouse.y;
    return mouse.bits & (MOUSE_LEFT | MOUSE_RIGHT);
}
```

Do not perform an extra header read or retain a second movement-only read.
Each new `controller_read_mouse()` call starts with a fresh latch and consumes
a complete report. Read once per game tick and use that report for both buttons
and movement.

`ControllerMouse` occupies four bytes under SDCC:

| Field  | Meaning                                                              |
| ------ | -------------------------------------------------------------------- |
| `bits` | Raw 16-bit header, including button, sensitivity, and signature bits |
| `x`    | Signed horizontal movement, from -127 through 127                    |
| `y`    | Signed vertical movement, from -127 through 127                      |

Axis decoding and sign conventions are unchanged from the old getters. The SDK
converts sign-magnitude data to signed values and normalizes negative zero.
There is no need to decode the sign bit again in the game.

Keep mouse buttons separate from the pad mask. Do not OR the raw `mouse.bits`
header into pad input: its signature and sensitivity bits are not pad buttons.
Mask with `MOUSE_LEFT | MOUSE_RIGHT`, then map those buttons to game actions.

### Handle the return value correctly

| Return value           | Meaning                                         | What the game should do                           |
| ---------------------- | ----------------------------------------------- | ------------------------------------------------- |
| `0`                    | Valid report, no movement                       | Still process mouse buttons                       |
| `1`, `2`, or `3`       | Valid report; Y, X, or both axes moved          | Process buttons and movement                      |
| `MOUSE_ERROR` (`0xFF`) | Invalid port, NULL output, or invalid signature | Ignore the report or clear your saved mouse input |

**Do not use `if (controller_read_mouse(...))` as a success check.** An error
is nonzero, while a stationary mouse can still click or release a button.
Compare against `MOUSE_ERROR` first, then process buttons even when movement is
zero.

An invalid signature clears the supplied report. An invalid port or NULL output
performs no I/O; do not assume an old buffer was cleared in those cases. Always
check the return value before using the fields.

## Poll a pad and mouse together

CLOCK and LATCH are physically shared. Polling the pad clocks both connected
devices, but the new mouse call starts with another latch and resets the serial
position. You do not need to realign it manually.

```text
Pad call:   latch both devices -> read the selected pad's 16 bits
Mouse call: latch both devices -> read the selected mouse's complete 32 bits
```

These are separate snapshots, not a single simultaneous pad-and-mouse report.
`controller_read_pads()` is for two 16-bit pad reports; it is not a combined
pad-and-mouse API.

The following example handles one pad and one mouse on either port, keeps
keyboard input, and computes press/release edges from game-owned state. Call
`game_input_init()` once, then `game_input_poll()` once per game tick. Pass a
valid `GameInput` output pointer.

**After**

```c
#include <zgdk/input.h>

#define NO_PORT 0xFF

typedef struct {
    uint16_t buttons;        /* Keyboard plus valid pad buttons. */
    uint16_t pressed;
    uint16_t mouse_buttons;
    uint16_t mouse_pressed;
    uint16_t mouse_released;
    int8_t mouse_dx;
    int8_t mouse_dy;
} GameInput;

static uint8_t pad_port = NO_PORT;
static uint8_t mouse_port = NO_PORT;
static uint16_t previous_buttons;
static uint16_t previous_mouse_buttons;

void game_input_poll(GameInput *input)
{
    uint16_t buttons = keyboard_read();
    uint16_t mouse_buttons = 0;
    int8_t mouse_dx = 0;
    int8_t mouse_dy = 0;

    if (pad_port != NO_PORT) {
        uint16_t pad = controller_read_pad(pad_port);
        if (controller_pad_valid(pad)) {
            buttons |= pad;
        }
    }

    if (mouse_port != NO_PORT) {
        ControllerMouse mouse;
        uint8_t result = controller_read_mouse(mouse_port, &mouse);
        if (result != MOUSE_ERROR) {
            mouse_buttons = mouse.bits & (MOUSE_LEFT | MOUSE_RIGHT);
            mouse_dx = mouse.x;
            mouse_dy = mouse.y;
        }
    }

    input->buttons = buttons;
    input->pressed = buttons & ~previous_buttons;
    input->mouse_buttons = mouse_buttons;
    input->mouse_pressed = mouse_buttons & ~previous_mouse_buttons;
    input->mouse_released = previous_mouse_buttons & ~mouse_buttons;
    input->mouse_dx = mouse_dx;
    input->mouse_dy = mouse_dy;

    previous_buttons = buttons;
    previous_mouse_buttons = mouse_buttons;
}

zos_err_t game_input_init(void)
{
    zos_err_t err = input_init(1);
    if (err != ERR_SUCCESS) {
        return err;
    }

    pad_port = NO_PORT;
    mouse_port = NO_PORT;
    previous_buttons = 0;
    previous_mouse_buttons = 0;

    if (controller_is_pad(SNES_PORT1)) {
        pad_port = SNES_PORT1;
    } else if (controller_is_pad(SNES_PORT2)) {
        pad_port = SNES_PORT2;
    }
    if (controller_is_mouse(SNES_PORT1)) {
        mouse_port = SNES_PORT1;
    } else if (controller_is_mouse(SNES_PORT2)) {
        mouse_port = SNES_PORT2;
    }

    if (mouse_port != NO_PORT) {
        /* A clone may reject this request; that does not make it unusable. */
        (void)controller_set_mouse_sensitivity(mouse_port, MOUSE_MEDIUM);
    }

    /* Seed edges from current state; discard movement present at startup. */
    GameInput initial;
    game_input_poll(&initial);
    return ERR_SUCCESS;
}
```

The example treats a failed mouse report as no buttons and no movement for that
tick. It detects a release when previously held mouse buttons disappear. Choose
a different disconnect/reconnect policy if your game requires it.

Raw polling bypasses the held-button suppression in `input_read()`. This example
uses its own edge tracking, including an initial baseline poll. Keep one owner
for your game's input polling rather than also calling `input_read()` elsewhere
in the same tick. Map `mouse_pressed` and `mouse_released` into your existing
accept, cancel, drag, and release events.

## Replace flushing and sensitivity handling

`controller_flush()` used to clear the SDK's saved pad reports and mouse deltas.
It did not drain a controller or wait for a button release. The SDK no longer
owns those reports, so clear the corresponding fields in your game state.

When suppressing held buttons across a menu or scene change, seed your saved
previous values from a fresh poll. Simply setting `previous_buttons` to zero
will make a still-held button appear newly pressed on the next tick. A baseline
mouse poll also consumes movement; discard it only when that is intentional.

Keep `input_flush()` if you use the high-level input API. It still drains the
keyboard and suppresses held pad buttons for subsequent `input_read()` calls.
It does not reset your own raw-input edge history.

The sensitivity APIs and `MOUSE_LOW`, `MOUSE_MEDIUM`, and `MOUSE_HIGH` values are
unchanged. The setter already handles first-use cycling and bounded retries.
Check its returned value against `MOUSE_ERROR` rather than testing it as a
boolean: `MOUSE_LOW` is a valid zero result.

Hyperkin mice may ignore software sensitivity changes and report low sensitivity
regardless of their physical setting. Use detection to identify a mouse; do not
reject it just because a sensitivity request failed. Its physical sensitivity
button remains the way to change that setting.

## Replace the deprecated input alias

`input_get()` remains available automatically through `zgdk/input.h` and
`zgdk.h`, but emits a warning naming its replacement, `input_read()`. No extra
include is needed. Including the header alone is quiet. The diagnostic works
with SDCC's bundled `sdcpp` preprocessor. If your build treats warnings as
errors, replace `input_get()` as part of the port.

`controller_read()` is a supported convenience alias for
`controller_read_pad(SNES_PORT1)` and emits no deprecation warning. There is no
need to rename a call that only needs the returned port-1 report.

Neither alias restores the removed global-state API. In particular,
`controller_read()` does not refresh port 2. There is no alias for
`controller_read_port()` or the old one-argument mouse function.

## Rebuild tests and verify the port

Update test headers and mocks to the actual signatures: a pad mock implements
`controller_read_pad(uint8_t port)`, and a mouse mock accepts both the port and
`ControllerMouse *` output. Replace detection mocks with the device-specific
functions. Tests that inspect `PORT1_bits`, `PORT2_bits`, or mouse globals should
inspect your game-owned reports instead. Update debugger scripts that call or
look up removed symbols as well.

Verify these cases before shipping:

- Build with SDCC using the matching SDK headers and rebuilt library; remove
  stale game objects and address any remaining deprecated calls.
- Check port 1 and port 2, absent devices, invalid signatures, and the game's
  chosen reconnect behavior. Invalid pad reports must not press every button.
- For two pads, use different button combinations on each device and confirm
  both players receive the right report.
- Click and release the mouse without moving it. Test movement and buttons
  together, both axes, and dragging/releasing across menu transitions.
- For a mixed setup, test both port assignments while the pad and mouse are
  active. Poll each configured device once per tick; remove old continuation
  reads and manual header reads.
- Test original and Hyperkin mice on hardware. A rejected sensitivity request
  must not disable an otherwise working Hyperkin mouse.

The SDK owns pad, mouse, and sensitivity timing, including the longer mouse
waits. Keep frame pacing separate from bit-level I/O and do not replace those
waits with a pad-speed loop. See [Input timing and verification](Input.md#controller-timing-and-verification)
for the timing profiles and current hardware qualification status.

Using only `controller_init()` and the single-pad API links no dual-pad reader
or mouse support. Using only `controller_init()` and mouse-specific APIs links
no pad entry point. Games using both share the transport helpers once;
automatically included deprecated headers add no wrapper code or report storage.

The mixed example deliberately keeps `input_init(1)`, which links the high-level
pad input module. For a lean keyboard-and-mouse-only game, initialize with
`keyboard_init()`, `keyboard_flush()`, and `controller_init()` instead of using
the high-level pad input API. Check each initialization result. Verify your
link map matches the APIs the game calls, not just the devices present at runtime.

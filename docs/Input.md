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

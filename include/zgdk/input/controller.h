/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once
#include <stdint.h>
#include <zos_errors.h>
#include <zgdk/input/button_map.h>

#define SNES_PORT1 0
#define SNES_PORT2 1
#define MOUSE_ERROR 0xFF

typedef enum { MOUSE_LOW = 0, MOUSE_MEDIUM = 1, MOUSE_HIGH = 2 } MouseSensitivity;
typedef struct { uint16_t port1, port2; } ControllerPads;
typedef struct { uint16_t bits; int8_t x, y; } ControllerMouse;

/* Configure PIO A; no report storage, mouse state, or polling code is linked. */
zos_err_t controller_init(void);
/* Raw 16-bit report, including device ID. Invalid port: 0xFFFF, no I/O.
 * Single-port reads sample only that data line. CLOCK/LATCH are shared wires. */
uint16_t controller_read_pad(uint8_t port);
/* One latch/clock transaction for both pads. NULL: no I/O. */
void controller_read_pads(ControllerPads *reports);
/* Complete 32-bit report. Movement flags: Y=1, X=2; MOUSE_ERROR on invalid
 * port, NULL output, or invalid signature. Invalid reports clear the output.
 * Axis sign-magnitude is decoded to signed -127..127, including negative zero. */
uint8_t controller_read_mouse(uint8_t port, ControllerMouse *report);
/* Detection consumes a report; mouse detection also drains the extra 16 bits
 * used by the original SNS-016/Hyperkin detection sequence. */
uint8_t controller_is_pad(uint8_t port);
uint8_t controller_is_mouse(uint8_t port);
/* Return the achieved sensitivity, or MOUSE_ERROR. First set on each port
 * cycles at least once; at most three cycles, including on fixed-speed clones. */
MouseSensitivity controller_get_mouse_sensitivity(uint8_t port);
uint8_t controller_set_mouse_sensitivity(uint8_t port, MouseSensitivity sensitivity);

#include "zgdk/input/controller/deprecated.h"

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

typedef enum {
    SNES_PAD   = 0,
    SNES_MOUSE = 1,
} ControllerType;

typedef enum {
    MOUSE_LOW    = 0,
    MOUSE_MEDIUM = 1,
    MOUSE_HIGH   = 2,
} MouseSensitivity;

extern uint16_t PORT1_bits;
extern uint16_t PORT2_bits;
extern int8_t MOUSE_y;
extern int8_t MOUSE_x;

zos_err_t controller_init(void);
zos_err_t controller_flush(void);

#define controller_read() controller_read_port(SNES_PORT1)
uint16_t controller_read_port(uint8_t port);
uint16_t controller_get(uint8_t port);

uint8_t controller_read_mouse(uint8_t port);
uint8_t controller_set_mouse_sensitivity(uint8_t port, MouseSensitivity s);
int8_t controller_get_mousey(void);
int8_t controller_get_mousex(void);

uint8_t controller_is(uint8_t port, ControllerType type);

#define controller_pressed_port1(button) (PORT1_bits & button)
#define controller_pressed_port2(button) (PORT2_bits & button)
#define controller_pressed(button) controller_pressed_port1(button)
#define controller_get_port1() (PORT1_bits)
#define controller_get_port2() (PORT2_bits)
#define controller_get_mousey() (MOUSE_y)
#define controller_get_mousex() (MOUSE_x)
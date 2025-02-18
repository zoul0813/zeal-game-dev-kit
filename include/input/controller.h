/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once

#include <stdint.h>
#include <zos_errors.h>
#include <zgdk/input/button_map.h>

extern uint16_t CONTROLLER_buttons;

zos_err_t controller_init(void);
zos_err_t controller_flush(void);

uint16_t controller_read(void);
static inline uint8_t controller_pressed(uint16_t button) {
    return CONTROLLER_buttons & button;
}
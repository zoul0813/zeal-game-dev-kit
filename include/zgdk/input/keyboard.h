#pragma once

#include <stdint.h>
#include <zos_errors.h>

extern uint16_t KEYBOARD_keys;

/**
 * @brief: Initialize the keyboard
 */
zos_err_t keyboard_init(void);

/**
 * @brief Drain queued keyboard events and clear held keys.
 * Incomplete release sequences remain pending for the next read.
 */
zos_err_t keyboard_flush(void);


/**
 * @brief Read held keyboard buttons as a 16-bit mask.
 * Opposing directions prefer the newest press, restoring the other on release.
 */
uint16_t keyboard_read(void);
uint8_t keyboard_pressed(uint16_t key);

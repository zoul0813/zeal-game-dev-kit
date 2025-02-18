#pragma once

#include <stdint.h>
#include <zos_errors.h>

extern uint16_t KEYBOARD_keys;

/**
 * @brief: Initialize the keyboard
 */
zos_err_t keyboard_init(void);

/**
 * @brief Flush the keyboard
 */
zos_err_t keyboard_flush(void);


/**
 * @brief Read from the keyboard, returns a 16-bit bitmask
 */
uint16_t keyboard_read(void);
static inline uint8_t keyboard_pressed(uint16_t key) {
    return KEYBOARD_keys & key;
}

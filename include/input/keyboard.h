#pragma once

#include <stdint.h>
#include <zos_errors.h>

extern uint16_t KEYBOARD_keys;

zos_err_t keyboard_init(void);
zos_err_t keyboard_flush(void);

uint16_t keyboard_read(void);
static inline uint8_t keyboard_pressed(uint16_t key) {
    return KEYBOARD_keys & key;
}

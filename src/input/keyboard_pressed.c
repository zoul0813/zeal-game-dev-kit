#include "zgdk/input/keyboard.h"

uint8_t keyboard_pressed(uint16_t key) {
    return KEYBOARD_keys & key;
}

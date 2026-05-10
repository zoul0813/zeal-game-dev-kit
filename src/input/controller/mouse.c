#include <zos_errors.h>
#include <zos_time.h>
#include <zos_keyboard.h>
#include "zgdk/input/button_map.h"
#include "zgdk/input/controller.h"
#include "pio.h"

/**
 * @brief Define some variables to access the PIO from C code
 */
__sfr __at(0xd0) IO_PIO_DATA_A;
__sfr __at(0xd2) IO_PIO_CTRL_A;

int8_t MOUSE_y      = 0; // nothing
int8_t MOUSE_x      = 0; // nothing

static void mouse_bit_delay(void) {
    NOP_WAIT(16);
}

static int8_t decode_mouse_axis(uint8_t raw) {
    uint8_t mag = raw & 0x7f;
    if(mag == 0) return 0;

    return (raw & 0x80) ? -(int8_t)mag : (int8_t)mag;
}

static uint8_t read_mouse_byte(uint8_t port) {
    uint8_t raw = 0;
    for(uint8_t i = 0; i < 8; i++) {
        raw <<= 1;
        CLOCK_ONCE();
        mouse_bit_delay();
        raw |= GET_DATA(port) == 0 ? 1 : 0;
    }
    return raw;
}

uint8_t controller_read_mouse(uint8_t port)
{
    uint8_t raw_y;
    uint8_t raw_x;

    mouse_bit_delay();
    mouse_bit_delay();
    mouse_bit_delay();
    mouse_bit_delay();

    raw_y = read_mouse_byte(port);

    mouse_bit_delay();

    raw_x = read_mouse_byte(port);

    MOUSE_y = decode_mouse_axis(raw_y);
    MOUSE_x = decode_mouse_axis(raw_x);

    if(MOUSE_x != 0 && MOUSE_y != 0) return 0x03;
    if(MOUSE_y != 0) return 0x01;
    if(MOUSE_x != 0) return 0x02;
    return 0;
}

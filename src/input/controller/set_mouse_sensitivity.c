#include <stdint.h>
#include "zgdk/input/button_map.h"
#include "zgdk/input/controller.h"
#include "pio.h"

#define MOUSE_SENSITIVITY_ERROR 0xFF

/**
 * @brief Define some variables to access the PIO from C code
 */
__sfr __at(0xd0) IO_PIO_DATA_A;
__sfr __at(0xd2) IO_PIO_CTRL_A;

static uint8_t MOUSE_initialized = 0;

static uint8_t mouse_valid_port(uint8_t port)
{
    return port == SNES_PORT1 || port == SNES_PORT2;
}

static uint8_t mouse_valid_sensitivity(MouseSensitivity s)
{
    return s == MOUSE_LOW || s == MOUSE_MEDIUM || s == MOUSE_HIGH;
}

static void mouse_report_delay(void)
{
    NOP();
    NOP();
}

static void mouse_latch_once(void)
{
    IO_PIO_DATA_A = (1 << IO_CLOCK) | (1 << IO_LATCH);
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    IO_PIO_DATA_A = 1 << IO_CLOCK;
    mouse_report_delay();
}

static void mouse_clock_once(void)
{
    IO_PIO_DATA_A = 0;
    mouse_report_delay();
    IO_PIO_DATA_A = 1 << IO_CLOCK;
    mouse_report_delay();
}

static uint16_t mouse_read_first_16_bits(uint8_t port)
{
    uint16_t bits;

    mouse_latch_once();

    bits = GET_DATA(port) == 0 ? 0x8000 : 0;
    for(uint8_t i = 0; i < 15; i++) {
        bits >>= 1;
        mouse_clock_once();
        bits |= GET_DATA(port) == 0 ? 0x8000 : 0;
    }

    return bits;
}

static uint8_t mouse_decode_sensitivity(uint16_t bits)
{
    switch(bits & MOUSE_SPEED) {
        case 0:        return MOUSE_LOW;
        case BUTTON_R: return MOUSE_MEDIUM;
        case BUTTON_L: return MOUSE_HIGH;
        default:       return MOUSE_SENSITIVITY_ERROR;
    }
}

static void mouse_cycle_sensitivity(void)
{
    IO_PIO_DATA_A = (1 << IO_CLOCK) | (1 << IO_LATCH);
    mouse_report_delay();

    IO_PIO_DATA_A = 1 << IO_LATCH;
    NOP();
    IO_PIO_DATA_A = (1 << IO_CLOCK) | (1 << IO_LATCH);
    mouse_report_delay();

    IO_PIO_DATA_A = 1 << IO_CLOCK;
    mouse_report_delay();
}

MouseSensitivity controller_get_mouse_sensitivity(uint8_t port)
{
    uint16_t bits;

    if(!mouse_valid_port(port)) {
        return (MouseSensitivity) MOUSE_SENSITIVITY_ERROR;
    }

    bits = mouse_read_first_16_bits(port);
    if((bits & MOUSE_ID) != MOUSE_ID) {
        return (MouseSensitivity) MOUSE_SENSITIVITY_ERROR;
    }

    return (MouseSensitivity) mouse_decode_sensitivity(bits);
}

uint8_t controller_set_mouse_sensitivity(uint8_t port, MouseSensitivity s)
{
    uint8_t current;
    uint8_t initialized_mask;

    if(!mouse_valid_port(port) || !mouse_valid_sensitivity(s)) {
        return MOUSE_SENSITIVITY_ERROR;
    }

    initialized_mask = 1 << port;
    current = (uint8_t) controller_get_mouse_sensitivity(port);
    if((MOUSE_initialized & initialized_mask) && current == (uint8_t) s) {
        return current;
    }

    /*
     * Original SNS-016 cycles low -> medium -> high -> low. Hyperkin-style
     * clones ignore software cycling and may always report low. Some original
     * mouse revisions need one cycle after power-up before reports are reliable,
     * so the first set for each port must cycle even when already at the target.
     */
    for(uint8_t i = 0; i < 3; i++) {
        mouse_cycle_sensitivity();
        current = (uint8_t) controller_get_mouse_sensitivity(port);
        if(current == (uint8_t) s) {
            MOUSE_initialized |= initialized_mask;
            return current;
        }
    }

    return MOUSE_SENSITIVITY_ERROR;
}

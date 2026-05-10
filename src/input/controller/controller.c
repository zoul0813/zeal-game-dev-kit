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

uint16_t PORT1_bits = 0; // nothing
uint16_t PORT2_bits = 0; // nothing

zos_err_t controller_flush(void)
{
    PORT1_bits = 0;
    PORT2_bits = 0;
    MOUSE_y    = 0;
    MOUSE_x    = 0;

    return ERR_SUCCESS;
}

zos_err_t controller_init(void)
{
    /**
     * Initialize the user port (port A) of the PIO
     * Set it to bit control mode so that each I/O can be controlled independently.
     */
    IO_PIO_CTRL_A = IO_PIO_BITCTRL;
    /**
     * After setting the port as a bit-controlled one, we need to give a bitmask of
     * pins that needs to be output (0) and input (1).
     * Set them all to output except DATA pins.
     */
    IO_PIO_CTRL_A = (1 << IO_DATA1) | (1 << IO_DATA2);
    /* Disable the interrupts for this port just in case it was activated */
    IO_PIO_CTRL_A = IO_PIO_DISABLE_INT;
    /**
     * Set the default value of each pin:
     *  - LATCH must be LOW (0)
     *  - CLOCK must be HIGH (1)
     * Set other pins to 0, not very important
     */
    IO_PIO_DATA_A = 1 << IO_CLOCK;

    return ERR_SUCCESS;
}

uint16_t controller_read_port(uint8_t port)
{
    LATCH_ONCE();

    // Now, the DATA lines contain the first button (B) state.

    PORT1_bits = GET_DATA(IO_DATA1) == 0 ? 0x8000 : 0;
    PORT2_bits = GET_DATA(IO_DATA2) == 0 ? 0x8000 : 0;
    // process the remaining 1 buttons (last 4 are unused)
    for (uint8_t i = 0; i < 15; ++i) {
        PORT1_bits = PORT1_bits >> 1;
        PORT2_bits = PORT2_bits >> 1;
        CLOCK_ONCE();                                       // pulse the clock
        PORT1_bits |= GET_DATA(IO_DATA1) == 0 ? 0x8000 : 0; // OR the current button
        PORT2_bits |= GET_DATA(IO_DATA2) == 0 ? 0x8000 : 0; // OR the current button
    }

    switch (port) {
        case SNES_PORT1: return PORT1_bits;
        case SNES_PORT2: return PORT2_bits;
    }
    return 0;
}


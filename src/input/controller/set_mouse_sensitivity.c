#include <stdint.h>
#include "zgdk/input/controller.h"
#include "pio.h"

/**
 * @brief Define some variables to access the PIO from C code
 */
__sfr __at(0xd0) IO_PIO_DATA_A;
__sfr __at(0xd2) IO_PIO_CTRL_A;

uint8_t controller_set_mouse_sensitivity(uint8_t port, MouseSensitivity s)
{
    (void) port; // unreferenced
    (void) s;    // unreferenced

    LATCH_ONCE();

    uint8_t i;
    for (i = 0; i < 15; ++i) {
        CLOCK_ONCE();
        NOP();
    }

    // delay ... ~1ms ((1.6us * 2) * 255 = 816us?
    // SDCC doesn't like (i=0;i<255;i++)
    // src/controller.c:191: warning 158: overflow in implicit constant conversion
    for(i = 255; i > 0; --i) {
        NOP_WAIT(2);
    }

    /**
     * A special sequence is used to rotate between the 3 modes. First, a normal 12us latch pulse is applied.
     * Next, the first 16 bits are read using normal button timings. Shortly after (about 1ms), 31 short latch
     * pulses (3.4uS) are sent, with the clock going low for 700ns during each latch pulse.
     *
     * For selecting a specific sensitivity, simply execute the special sequence until bits 11 and 12 are as desired.
     */

    for (i = 0; i < 31; i++) {
        IO_PIO_DATA_A = (1 << IO_CLOCK) | (1 << IO_LATCH); // latch set
        NOP();
        IO_PIO_DATA_A = (1 << IO_LATCH);                   // clock release
        NOP();
        IO_PIO_DATA_A = 0;
        NOP();
    }

    IO_PIO_DATA_A = 1 << IO_CLOCK; // clock set

    return 0; // the current sensitivity reading?
}

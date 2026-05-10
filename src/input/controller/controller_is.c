#include <stdint.h>
#include "zgdk/input/controller.h"

uint8_t controller_is(uint8_t port, ControllerType type)
{
    controller_flush();
    uint16_t bits = controller_read_port(port);

    switch (type) {
        case SNES_PAD: {
            return (bits & 0xF000) == 0;
        }
        case SNES_MOUSE:
            controller_read_mouse(port); // read and discard the X/Y
            // second read, to get bytes 5-6
            // SNES should report 0xFFFF
            // Hyperkin should report 0x8000
            controller_read_mouse(port);
            return (bits & 0xF000) == MOUSE_ID;
        default: return 0;
    }
}
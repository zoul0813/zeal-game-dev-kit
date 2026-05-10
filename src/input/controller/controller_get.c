#include <stdint.h>
#include "zgdk/input/controller.h"

uint16_t controller_get(uint8_t port)
{
    switch (port) {
        case SNES_PORT1: return PORT1_bits;
        case SNES_PORT2: return PORT2_bits;
    }
    return 0;
}
#pragma once
#include "internal.h"
#include "zgdk/inline.h"

#define OFFSET_Y 3

INLINE uint8_t hiscore_offset_x(void) {
    return (hiscore_config->width - 10) / 2;
}

#pragma once
#include <stdint.h>
typedef struct {
    uint16_t y, x;
    uint8_t tile, flags;
    uint16_t options;
} gfx_sprite;

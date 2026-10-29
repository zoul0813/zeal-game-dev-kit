#pragma once
#include <stdint.h>
typedef enum { GFX_SUCCESS, GFX_INVALID_ARG } gfx_error;
typedef struct { uint8_t video_mode; uint8_t bpp; } gfx_context;
typedef struct {
    uint8_t compression;
    uint16_t from_byte;
    uint8_t pal_offset;
    uint8_t opacity;
} gfx_tileset_options;

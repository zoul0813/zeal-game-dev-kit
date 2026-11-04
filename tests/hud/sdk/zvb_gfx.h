#pragma once
#include <stdint.h>
typedef enum { GFX_SUCCESS, GFX_INVALID_ARG } gfx_error;
typedef struct { uint8_t backup_page; } gfx_context;
gfx_error gfx_tilemap_load(gfx_context *, void *, uint8_t, uint8_t, uint8_t, uint8_t);

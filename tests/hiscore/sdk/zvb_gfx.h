#pragma once
#include <stdint.h>
typedef enum { GFX_SUCCESS, GFX_INVALID_ARG } gfx_error;
typedef struct { uint8_t backup_page; } gfx_context;
void gfx_wait_vblank(gfx_context *);
void gfx_wait_end_vblank(gfx_context *);

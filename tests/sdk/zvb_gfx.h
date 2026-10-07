#pragma once
#include <stdint.h>
#include <stddef.h>
typedef uint8_t gfx_error;
typedef struct {uint8_t backup_page;} gfx_context;
#define GFX_SUCCESS 0
#define GFX_INVALID_ARG 2
gfx_error gfx_tilemap_load(gfx_context*,void*,uint8_t,uint8_t,uint8_t,uint8_t);
gfx_error gfx_tilemap_place(gfx_context*,uint8_t,uint8_t,uint8_t,uint8_t);

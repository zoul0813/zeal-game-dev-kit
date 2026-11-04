#ifndef TEST_ZVB_GFX_H
#define TEST_ZVB_GFX_H
#include <stdint.h>
typedef uint8_t gfx_error;
#define GFX_SUCCESS 0
#define GFX_FAILURE 1
#define GFX_INVALID_ARG 2
#define TILESET_COMP_NONE 0
typedef struct { uint8_t video_mode,bpp,alpha_idx,virtual_page,backup_page; } gfx_context;
typedef struct { uint8_t compression; uint16_t from_byte; uint8_t pal_offset,opacity; } gfx_tileset_options;
gfx_error gfx_wait_vblank(gfx_context*);
gfx_error gfx_wait_end_vblank(gfx_context*);
gfx_error gfx_tileset_load_none(gfx_context*,uint8_t*,uint16_t,uint16_t,uint8_t,uint8_t);
gfx_error gfx_tilemap_place(gfx_context*,uint8_t,uint8_t,uint8_t,uint8_t);
gfx_error gfx_tilemap_load(gfx_context*,void*,uint8_t,uint8_t,uint8_t,uint8_t);
#endif

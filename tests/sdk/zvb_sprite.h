#pragma once
#include "zvb_gfx.h"
#define GFX_SPRITES_COUNT 128
typedef struct {uint16_t y,x;uint8_t tile,flags;uint16_t options;} gfx_sprite;
gfx_error gfx_sprite_render_array(gfx_context*,uint8_t,const gfx_sprite*,uint8_t);

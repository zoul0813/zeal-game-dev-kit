#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>

#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE_HEIGHT       16
#define SPRITE_WIDTH        16

gfx_error sprites_register_arena(gfx_sprite* arena, uint8_t size);
gfx_sprite* sprites_register(gfx_sprite sprite);
gfx_error sprites_render(gfx_context* ctx);
#endif
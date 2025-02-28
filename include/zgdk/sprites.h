#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>
#include "zgdk/types.h"

#ifndef SPRITES_H
#define SPRITES_H

gfx_error sprites_register_arena(gfx_sprite* arena, uint8_t size);
gfx_sprite* sprites_register(gfx_sprite sprite);
gfx_error sprites_render(gfx_context* ctx);

inline void sprite_get_point(gfx_sprite *sprite, Point *point);
inline void sprite_set_point(gfx_sprite *sprite, Point *point);
inline void sprite_set_xy(gfx_sprite *sprite, uint16_t x, uint16_t y);
#endif
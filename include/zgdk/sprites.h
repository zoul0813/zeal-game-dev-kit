#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>
#include "zgdk/types.h"

#ifndef SPRITES_H
#define SPRITES_H

/* size is an element count from 1 through GFX_SPRITES_COUNT, not bytes. */
gfx_error sprites_register_arena(gfx_sprite* arena, uint8_t size);
gfx_sprite* sprites_register(gfx_sprite sprite);
/* Invalidates registered pointers. Render once to hide old hardware slots. */
void sprites_deregister(void);
gfx_error sprites_render(gfx_context* ctx);

void sprite_get_point(gfx_sprite *sprite, Point *point);
void sprite_set_point(gfx_sprite *sprite, Point *point);
void sprite_set_xy(gfx_sprite *sprite, uint16_t x, uint16_t y);
#endif

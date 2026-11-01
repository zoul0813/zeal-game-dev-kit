#pragma once
#include "zgdk/sprites.h"

/* Shared private arena state; point helpers do not depend on it. */
extern gfx_sprite *sprites_arena;
extern uint8_t sprites_arena_size;
extern uint8_t sprites_registered;
extern uint8_t sprites_rendered;

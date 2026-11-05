#pragma once
#include "zgdk/sprites.h"

/* Shared private arena state; point helpers do not depend on it. */
extern gfx_sprite *sprites_arena;
extern uint8_t sprites_arena_size;
extern uint8_t sprites_registered;
extern uint8_t sprites_rendered;

/* Default DMA clock (0x56): 32-byte chunks pass release-phase sweeps;
 * 64/128-byte chunks lose releases. Slower clocks need renewed validation. */
#define SPRITES_DMA_CHUNK_BYTES 32

/* Private transfer plan, rebuilt when an arena or its physical mapping changes. */
void sprites_dma_prepare(void);
uint8_t sprites_dma_upload(void);

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

/* Lazily probe DMA and prepare a plan on the first explicit DMA upload.
 * Rebuild when the arena, its capacity, or its physical mapping changes. */
uint8_t sprites_dma_upload(uint8_t start, uint8_t count);

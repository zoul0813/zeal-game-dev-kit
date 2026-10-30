#ifndef TEST_ZVB_GFX_H
#define TEST_ZVB_GFX_H
#include <stdint.h>
typedef struct { uint8_t unused; } gfx_context;
typedef enum { GFX_SUCCESS=0, GFX_INVALID_ARG=17 } gfx_error;
gfx_error gfx_tilemap_load(gfx_context *, void *, uint8_t, uint8_t, uint8_t, uint8_t);
gfx_error gfx_tilemap_place(gfx_context *, uint8_t, uint8_t, uint8_t, uint8_t);
#endif

#include <stdint.h>
#include <zvb_gfx.h>

extern uint8_t _tileset_ztp_start;
extern uint8_t _tileset_ztp_end;
extern uint8_t _tileset_zts_start;
extern uint8_t _tileset_zts_end;

gfx_error load_palette(gfx_context* ctx);
gfx_error load_tileset(gfx_context* ctx, gfx_tileset_options* options);

#ifndef ZGDK_HUD_SPRITE_INTERNAL_H
#define ZGDK_HUD_SPRITE_INTERNAL_H
#include "zgdk/hud/sprite.h"
/* Hardware Y includes a +16 origin bias. This parks the top at screen Y=496,
 * below the maximum 480-line display, independent of sprite height/options. */
#define HUD_SPRITE_HIDDEN_Y 512
uint8_t hud_sprite_text_length(const hud_sprite_text_t *text, const char *str);
#endif

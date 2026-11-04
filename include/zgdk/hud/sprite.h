#ifndef ZGDK_HUD_SPRITE_H
#define ZGDK_HUD_SPRITE_H
#include <stddef.h>
#include <stdint.h>
#include <zvb_sprite.h>

/* Caller-owned sprites, already registered if using the SDK arena. This view
 * owns no storage. spacing is the horizontal pixel advance. Text uses the same
 * ASCII_MAP / ascii_map() font mapping as the tilemap HUD helpers. */
typedef struct {
    gfx_sprite *sprites;
    uint8_t capacity;
    uint8_t spacing;
} hud_sprite_text_t;

/* Coordinates are screen pixels; helpers add the hardware +16 origin bias.
 * Only x/y/tile change; flags/options remain caller-controlled. No allocation,
 * registration or VRAM upload. Spaces, non-printable bytes and unused slots
 * are parked below the screen at hardware y=512, while still consuming their
 * character spacing. This hides either sprite height in all supported modes.
 * A NULL string is empty; a NULL view/range or zero capacity is a no-op.
 * The view must describe a valid writable range of capacity sprites.
 * put_s reads at most capacity bytes, stopping at NUL; nput_s consumes exactly
 * min(len, capacity) bytes, without requiring a terminator. */
void hud_sprite_put_s(const hud_sprite_text_t *text, const char *str, int16_t x, int16_t y);
void hud_sprite_nput_s(const hud_sprite_text_t *text, const char *str, uint8_t len, int16_t x, int16_t y);
/* Center the truncated string's spacing cells on x. Trailing spaces count.
 * The resulting left position must fit int16_t. */
void hud_sprite_put_s_centered(const hud_sprite_text_t *text, const char *str, int16_t x, int16_t y);
/* Hide any caller-owned contiguous range, independently of font/layout. */
void hud_sprite_hide(gfx_sprite *sprites, uint8_t count);
#endif

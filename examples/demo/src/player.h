#include <stdint.h>
#include <zvb_sprite.h>
#include <zgdk.h>
#include "main.h"

#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_SPEED (2U)

typedef struct {
        uint8_t sprite_index;
        gfx_sprite sprite_head;
        gfx_sprite sprite_body;
        Direction direction;
} player_t;

extern player_t player;

error player_init(void);
error player_deinit(void);
void player_move(void);
void player_draw(void);

#endif
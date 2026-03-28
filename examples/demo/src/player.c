#include <zos_errors.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>
#include "player.h"
#include "main.h"

player_t player;

error player_init(void)
{
    gfx_error err;
    player.sprite_index = 0;

    player.sprite_body.tile  = 2;
    player.sprite_body.x     = SCREEN_WIDTH / 2;
    player.sprite_body.y     = SCREEN_HEIGHT / 2;
    player.sprite_body.flags = SPRITE_NONE;
    err                      = gfx_sprite_render(&vctx, player.sprite_index, &player.sprite_body);
    if (err != GFX_SUCCESS)
        return err;

    player.sprite_head.tile  = 1;
    player.sprite_head.x     = player.sprite_body.x;
    player.sprite_head.y     = player.sprite_body.y - SPRITE_HEIGHT;
    player.sprite_head.flags = SPRITE_NONE;
    err                      = gfx_sprite_render(&vctx, player.sprite_index + 1, &player.sprite_head);
    return err;
}

error player_deinit(void)
{
    gfx_error err;
    err = gfx_sprite_set_tile(&vctx, player.sprite_index, EMPTY_TILE);
    err = gfx_sprite_set_tile(&vctx, player.sprite_index + 1, EMPTY_TILE);
    return err;
}

void player_move(void)
{
    uint16_t x  = player.sprite_body.x;
    uint16_t y  = player.sprite_body.y;
    x          += (player.direction.x * PLAYER_SPEED);
    y          += (player.direction.y * PLAYER_SPEED);

    switch (player.direction.x) {
        case DIRECTION_LEFT: {
            player.sprite_head.flags = SPRITE_FLIP_X;
            player.sprite_body.flags = SPRITE_FLIP_X;
        } break;
        case DIRECTION_RIGHT: {
            player.sprite_head.flags = SPRITE_NONE;
            player.sprite_body.flags = SPRITE_NONE;
        } break;
    }

    if(x < SPRITE_WIDTH) x = SPRITE_WIDTH;
    else if(x > SCREEN_WIDTH) x = SCREEN_WIDTH;
    if(y < SPRITE_HEIGHT) y = SPRITE_HEIGHT;
    else if(y > SCREEN_HEIGHT + SPRITE_HEIGHT) y = SCREEN_HEIGHT + SPRITE_HEIGHT;

    player.sprite_body.x = x;
    player.sprite_body.y = y;
    player.sprite_head.x = x;
    player.sprite_head.y = y - SPRITE_HEIGHT;
}

void player_draw(void)
{
    // ignore errors
    gfx_sprite_render(&vctx, player.sprite_index, &player.sprite_head);
    gfx_sprite_render(&vctx, player.sprite_index + 1, &player.sprite_body);
}
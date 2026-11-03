#include "zgdk/physics/platformer.h"

uint8_t platformer_update(body_t *body, const platformer_config_t *config, platformer_state_t *state, int8_t dir_x, uint8_t jump_held, uint8_t jump_pressed) {
    uint8_t jumped = 0;

    // run
    if (dir_x > 0) {
        body->vx += config->accel;
        if (body->vx > config->max_run) body->vx = config->max_run;
    } else if (dir_x < 0) {
        body->vx -= config->accel;
        if (body->vx < -config->max_run) body->vx = -config->max_run;
    } else if (body->vx > 0) {
        body->vx -= config->decel;
        if (body->vx < 0) body->vx = 0;
    } else if (body->vx < 0) {
        body->vx += config->decel;
        if (body->vx > 0) body->vx = 0;
    }

    // coyote time + jump buffer
    if (body->state & BODY_ON_GROUND) {
        state->coyote = config->coyote + 1;
        state->air_left = config->air_jumps;
    } else if (state->coyote) {
        state->coyote--;
    }
    if (jump_pressed) state->buffer = config->buffer + 1;
    else if (state->buffer) state->buffer--;

    if (state->buffer && (state->coyote || (body->state & BODY_PASS_ONEWAY))) {
        body->vy = config->jump_vel;
        state->coyote = 0;
        state->buffer = 0;
        body->state &= ~(BODY_ON_GROUND | BODY_PASS_ONEWAY);
        jumped = 1;
    } else if (jump_pressed && state->air_left) {
        body->vy = config->jump_vel;
        state->air_left--;
        state->buffer = 0;
        jumped = 1;
    }

    // variable height: releasing jump cuts the rise short
    if (!jump_held && body->vy < config->jump_cut) body->vy = config->jump_cut;

    return jumped;
}

#ifndef ZGDK_PHYSICS_PLATFORMER_H
#define ZGDK_PHYSICS_PLATFORMER_H
#include <zgdk/physics/tilemap.h>
#include <zgdk/types.h>

/* Optional controller state: 3 bytes, allocated only for controlled bodies.
 * Zero-initialize before use. All speeds/accelerations are 12.4 per tick. */
typedef struct { uint8_t coyote, buffer, air_left; } platformer_state_t;

typedef struct {
    int16_t gravity;     // added to vy each frame
    int16_t max_fall;    // vy clamp
    int16_t jump_vel;    // vy on jump (negative)
    int16_t jump_cut;    // vy is raised to this when jump is released early (negative, closer to 0)
    int16_t accel;       // vx change per frame while a direction is held
    int16_t decel;       // vx change per frame with no direction
    int16_t max_run;     // vx clamp
    uint8_t coyote;      // frames a jump is still allowed after leaving the ground
    uint8_t buffer;      // frames a jump press is remembered before landing
    uint8_t air_jumps;   // extra jumps in the air (1 = double jump)
} platformer_config_t;

/* Run/jump input updates velocity and timers; returns 1 when a jump starts. */
uint8_t platformer_update(body_t *body, const platformer_config_t *config, platformer_state_t *state,
    int8_t dir_x, uint8_t jump_held, uint8_t jump_pressed);
/* Apply configured gravity unless climbing, then body_step(). */
void platformer_move(body_t *body, const platformer_config_t *config);
/* Call before platformer_update. Uses standard TF_LADDER/TF_ONEWAY flags.
 * Uses BODY_PASS_ONEWAY to mark climbing and permit ladder-top descent. */
uint8_t platformer_ladder(body_t *body, uint8_t up, uint8_t down, int8_t dir_x, int16_t climb_speed);
INLINE uint8_t platformer_is_climbing(const body_t *body) { return body->state & BODY_PASS_ONEWAY; }
/* Clear buffered/coyote jumps on room changes/respawn. Preserve air jumps,
 * matching reposition behavior; landing replenishes them from the config. */
INLINE void platformer_reset(platformer_state_t *state) { state->coyote = 0; state->buffer = 0; }
INLINE void platformer_set_pos(body_t *body, platformer_state_t *state, int16_t px, int16_t py) {
    body_set_pos(body, px, py);
    platformer_reset(state);
}
INLINE void platformer_init(body_t *body, platformer_state_t *state, int16_t px, int16_t py, uint8_t w, uint8_t h) {
    body_init(body, px, py, w, h);
    platformer_reset(state);
    state->air_left = 0;
}
#endif

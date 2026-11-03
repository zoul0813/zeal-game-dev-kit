#include "zgdk/physics/platformer.h"

void platformer_move(body_t *body, const platformer_config_t *config) {
    if (!(body->state & BODY_PASS_ONEWAY)) {
        body_apply_gravity(body, config->gravity, config->max_fall);
    }
    body_step(body);
}

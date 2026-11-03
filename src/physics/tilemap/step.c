#include "internal.h"
#include "../../tilemap/flags/cache.h"

void body_step(body_t *body) {
    body->state &= BODY_PASS_ONEWAY;
    if (!tileflags_ready()) {
        // no tilemap/flags registered: move freely
        body->x += body->vx;
        body->y += body->vy;
        body->touched = 0;
        return;
    }
    body_move_x(body);
    body_move_y(body);
    body_ground_check(body);
    body_touch(body);
}

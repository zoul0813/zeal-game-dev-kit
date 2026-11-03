#include "internal.h"

/* A body resting on a tile top needs a probe just below its feet. */
void body_ground_check(body_t *body) {
    int16_t bottom;
    uint8_t flags;
    if (body->vy < 0 || (body->state & BODY_ON_GROUND)) return;
    body_edges(body);
    bottom = body_edge_bottom + 1;        // first row below the feet
    if (bottom & 15) return;              // feet aren't on a tile boundary
    flags = body_probe_row(bottom, body_edge_left, body_edge_right);
    if ((flags & TF_SOLID) || ((flags & TF_ONEWAY) && !(body->state & BODY_PASS_ONEWAY))) {
        body->y = PHYSICS_FP(bottom - body->h);
        body->vy = 0;
        body->state |= BODY_ON_GROUND;
    }
}

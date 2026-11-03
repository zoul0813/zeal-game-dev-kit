#include "internal.h"

void body_move_y(body_t *body) {
    int16_t old_bottom;
    uint8_t flags;
    if (body->vy == 0) return;
    body_edges(body);
    old_bottom = body_edge_bottom;
    body->y += body->vy;
    body_edges(body);
    if (body->vy > 0) {
        flags = body_probe_row(body_edge_bottom, body_edge_left, body_edge_right);
        // one-way tiles only catch a body whose feet were above the tile last frame
        // one-way tiles don't catch a climbing body, so it can climb down through a ladder top
        if ((flags & TF_SOLID) ||
            ((flags & TF_ONEWAY) && !(body->state & BODY_PASS_ONEWAY) && (old_bottom >> 4) < (body_edge_bottom >> 4))) {
            body->y = PHYSICS_FP((body_edge_bottom & ~15) - body->h);
            body->vy = 0;
            body->state |= BODY_ON_GROUND;
        }
    } else {
        if (body_probe_row(body_edge_top, body_edge_left, body_edge_right) & TF_SOLID) {
            body->y = PHYSICS_FP((body_edge_top & ~15) + 16);
            body->vy = 0;
            body->state |= BODY_HIT_CEIL;
        }
    }
}

#include "internal.h"

void body_move_x(body_t *body) {
    if (body->vx == 0) return;
    body->x += body->vx;
    body_edges(body);
    if (body->vx > 0) {
        if (body_probe_column(body_edge_right, body_edge_top, body_edge_bottom) & TF_SOLID) {
            body->x = PHYSICS_FP((body_edge_right & ~15) - body->w);
            body->vx = 0;
            body->state |= BODY_HIT_WALL;
        }
    } else {
        if (body_probe_column(body_edge_left, body_edge_top, body_edge_bottom) & TF_SOLID) {
            body->x = PHYSICS_FP((body_edge_left & ~15) + 16);
            body->vx = 0;
            body->state |= BODY_HIT_WALL;
        }
    }
}

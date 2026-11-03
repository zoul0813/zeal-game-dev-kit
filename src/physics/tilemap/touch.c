#include "internal.h"

void body_touch(body_t *body) {
    int16_t cx, cy;
    uint8_t center;
    body->touched = 0;
    if (!body_touch_mask) return;
    body_edges(body);
    cx = body_edge_left + (body->w >> 1);
    cy = body_edge_top + (body->h >> 1);
    // hazards use a slightly inset box so grazing a spike corner is forgiven
    center = body_probe_row(cy, cx, cx);
    body->touched = (center |
        body_probe_row(cy, body_edge_left + 2, body_edge_right - 2) |
        body_probe_column(cx, body_edge_top + 2, body_edge_bottom - 2)) & body_touch_mask;
    if (body->touched & TF_HAZARD) body->state |= BODY_IN_HAZARD;
    if (center & body_touch_mask & TF_LADDER) body->state |= BODY_ON_LADDER;
}

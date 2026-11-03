#include "zgdk/physics/body.h"

void body_init(body_t *body, int16_t px, int16_t py, uint8_t w, uint8_t h) {
    body->vx = 0;
    body->vy = 0;
    body->w = w;
    body->h = h;
    body->state = 0;
    body_set_pos(body, px, py);
}

void body_set_pos(body_t *body, int16_t px, int16_t py) {
    body->x = PHYSICS_FP(px);
    body->y = PHYSICS_FP(py);
    body->state = 0;
    body->touched = 0;
}

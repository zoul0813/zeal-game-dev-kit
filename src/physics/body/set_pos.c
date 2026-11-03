#include "zgdk/physics/body.h"

void body_set_pos(body_t *body, int16_t px, int16_t py) {
    body->x = PHYSICS_FP(px);
    body->y = PHYSICS_FP(py);
    body->state = 0;
    body->touched = 0;
}

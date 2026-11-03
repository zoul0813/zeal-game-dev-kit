#include "zgdk/physics/body.h"

uint8_t body_overlap(const body_t *a, const body_t *b) {
    int16_t ax = PHYSICS_FP_INT(a->x), ay = PHYSICS_FP_INT(a->y);
    int16_t bx = PHYSICS_FP_INT(b->x), by = PHYSICS_FP_INT(b->y);
    return (ax < bx + b->w) && (bx < ax + a->w) && (ay < by + b->h) && (by < ay + a->h);
}

#include "zgdk/physics/platformer.h"
#include "zgdk/physics/tileflags.h"

uint8_t platformer_ladder(body_t *body, uint8_t up, uint8_t down, int8_t dir_x, int16_t climb_speed) {
    int16_t cx = PHYSICS_FP_INT(body->x) + (body->w >> 1);
    int16_t top = PHYSICS_FP_INT(body->y);
    uint8_t center = tileflags_at(cx, top + (body->h >> 1));
    uint8_t below = tileflags_at(cx, top + body->h);

    if (up && (center & TF_LADDER)) body->state |= BODY_PASS_ONEWAY;
    if (down && ((center | below) & TF_LADDER)) body->state |= BODY_PASS_ONEWAY;
    if (!((center | below) & TF_LADDER)) body->state &= ~BODY_PASS_ONEWAY;
    if (!(body->state & BODY_PASS_ONEWAY)) return 0;

    body->vy = up ? -climb_speed : down ? climb_speed : 0;
    if (dir_x == DIRECTION_NONE) body->vx = 0;
    return 1;
}

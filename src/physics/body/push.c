#include <zgdk/utils/math.h>
#include <zgdk/physics/body.h>

void body_push(body_t *body, int16_t dvx, int16_t dvy, int16_t limit) {
    body->vx = clamp_i16(body->vx + dvx, -limit, limit);
    body->vy = clamp_i16(body->vy + dvy, -limit, limit);
}

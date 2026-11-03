#include <assert.h>
#include <stdio.h>
#include <zgdk/physics/body.h>
_Static_assert(sizeof(body_t) == 12, "generic bodies must not contain controller state");
int main(void) {
    body_t ship, shot;
    body_init(&ship, 100, 100, 16, 16);
    body_init(&shot, 110, 105, 2, 2);
    assert(body_overlap(&ship, &shot));
    body_accelerate(&ship, PHYSICS_FP(1), -PHYSICS_FP_ONE / 2);
    body_integrate(&ship);
    assert(ship.x == PHYSICS_FP(101) && ship.y == PHYSICS_FP(100) - PHYSICS_FP_ONE / 2);
    body_set_pos(&ship, 200, 200);
    assert(ship.vx == PHYSICS_FP(1) && ship.vy == -PHYSICS_FP_ONE / 2);
    assert(!body_overlap(&ship, &shot));
    body_apply_gravity(&shot, PHYSICS_FP_ONE / 2, PHYSICS_FP(2));
    assert(shot.vy == PHYSICS_FP_ONE / 2);
    shot.vy = PHYSICS_FP(2);
    body_apply_gravity(&shot, PHYSICS_FP_ONE / 2, PHYSICS_FP(2));
    assert(shot.vy == PHYSICS_FP(2));
    puts("free bodies: passed without a tilemap, cache or platformer controller");
    return 0;
}

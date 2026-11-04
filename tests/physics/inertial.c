#include <assert.h>
#include <stdio.h>
#include <zgdk/physics/inertial.h>

int main(void) {
    body_t b;
    const inertial_config_t c = {3, 2, 40, -2, 2};
    inertial_config_t coast = {3, 0, 40, 0, 0};
    int i, x, y;
    body_init(&b, 10, 20, 12, 12);
    for (y = -1; y <= 1; ++y) for (x = -1; x <= 1; ++x) {
        b.vx = b.vy = 0;
        for (i = 0; i < 40; ++i) inertial_update(&b, &c, x, y);
        assert(b.vx == x * ((x && y) ? 30 : 40));
        assert(b.vy == (y ? y * ((x && y) ? 30 : 40) : -2));
        assert(body_px(&b) == 10 && body_py(&b) == 20);
    }
    b.vx = 40; b.vy = -40;
    for (i = 0; i < 40; ++i) inertial_update(&b, &c, 0, 0);
    assert(b.vx == 0 && b.vy == -2);
    body_push(&b, 100, -100, 80);
    assert(b.vx == 80 && b.vy == -80);
    inertial_update(&b, &c, -1, 1);
    assert(b.vx == 78 && b.vy == -78);
    for (i = 0; i < 40; ++i) inertial_update(&b, &c, 1, -1);
    assert(b.vx == 30 && b.vy == -30);
    b.vx = 40; b.vy = -40;
    inertial_update(&b, &coast, 0, 0);
    assert(b.vx == 40 && b.vy == -40);
    body_push(&b, 100, -100, 80);
    inertial_update(&b, &coast, -1, 1);
    assert(b.vx == 80 && b.vy == -80);
    body_push(&b, -300, 300, 60);
    assert(b.vx == -60 && b.vy == 60);
    body_push(&b, 0, 0, 0);
    assert(b.vx == 0 && b.vy == 0);
    /* Drift outside the cap must settle at the cap, including a zero cap.
     * Exercise both signs and starting velocities below/at/above the cap. */
    for (y = -1; y <= 1; y += 2) for (x = 0; x <= 1; ++x) {
        inertial_config_t drift = {3, 2, x * 40, y * 60, 2};
        int start;
        for (start = 0; start <= 80; start += 40) {
            b.vx = 0;
            b.vy = y * start;
            for (i = 0; i < 100; ++i) {
                int was_within_cap = b.vy >= -drift.max_speed && b.vy <= drift.max_speed;
                inertial_update(&b, &drift, 0, 0);
                if (was_within_cap)
                    assert(b.vy >= -drift.max_speed && b.vy <= drift.max_speed);
            }
            assert(b.vy == y * drift.max_speed);
        }
    }
    puts("inertial controller and bounded impulses: passed without collision/platformer linkage");
    return 0;
}

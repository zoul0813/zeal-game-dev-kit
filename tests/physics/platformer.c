#include <zgdk/physics/platformer.h>
/* Platformer behavior regression tests, migrated from the game core. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zgdk/tilemap/flags.h>
#include <zgdk/physics/body.h>

static const char *MAP[15] = {
    "####################",
    "#..................#",
    "#..................#",
    "#..................#",
    "#..................#",
    "#..................#",
    "#..................#",
    "#..................#",
    "#..........L.......#",
    "#..........H.......#",
    "#...===####H.......#",
    "#..........H.......#",
    "#..........H.......#",
    "####################",
    "####################",
};

#include "zgdk/tilemap.h"

TILEFLAGS_DEFINE_CACHE(40);
Tilemap *_TILEMAP;
static Tilemap tilemap;
static uint8_t tiles[20 * 15];

static void load_map(void) {
    int x, y;
    for (y = 0; y < 15; y++) for (x = 0; x < 20; x++) tiles[y * 20 + x] = (uint8_t)MAP[y][x];
    tilemap.tiles = tiles;
    tilemap.rect.w = 20;
    tilemap.rect.h = 15;
    _TILEMAP = &tilemap;
}

static uint8_t flags[256];
static const platformer_config_t PHYS = { 4, 96, -70, -24, 4, 6, 28, 5, 6, 1 };
static int fails = 0;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

static void independent_controllers(void) {
    body_t a, b;
    platformer_state_t first = {0}, second = {0};
    platformer_init(&a, &first, 0, 0, 12, 12);
    platformer_init(&b, &second, 0, 0, 12, 12);
    first.air_left = second.air_left = 1;
    CHECK(platformer_update(&a, &PHYS, &first, 0, 1, 1), "first air jump");
    CHECK(first.air_left == 0 && second.air_left == 1 && b.vy == 0, "separate controller state");
    second.coyote = 4; second.buffer = 3;
    b.vx = 17; b.vy = -23;
    platformer_set_pos(&b, &second, 32, 64);
    CHECK(second.coyote == 0 && second.buffer == 0 && second.air_left == 1, "teleport timers");
    CHECK(b.vx == 17 && b.vy == -23, "teleport velocity");
}

int main(void) {
    independent_controllers();
    body_t b;
    platformer_state_t controller = {0};
    int i, grounded = 0;
    flags['#'] = TF_SOLID; flags['='] = TF_ONEWAY; flags['H'] = TF_LADDER; flags['L'] = TF_LADDER | TF_ONEWAY;
    tileflags_register(flags);
    load_map();

    platformer_init(&b, &controller, 34, 12 * 16 - 14, 12, 30);
    for (i = 0; i < 30; i++) { platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); if (b.state & BODY_ON_GROUND) grounded++; }
    CHECK(body_py(&b) == 13 * 16 - 30, "rest y %d", body_py(&b));
    CHECK(grounded >= 28, "on ground %d/30 frames while standing", grounded);

    for (i = 0; i < 40; i++) { platformer_update(&b, &PHYS, &controller, 1, 0, 0); platformer_move(&b, &PHYS); }
    CHECK(body_px(&b) > 34 + 50, "ran right to x %d", body_px(&b));

    // walk into the right wall
    for (i = 0; i < 200; i++) { platformer_update(&b, &PHYS, &controller, 1, 0, 0); platformer_move(&b, &PHYS); }
    CHECK(body_px(&b) == 19 * 16 - 12, "stopped at wall x %d", body_px(&b));

    // jump: rises then lands
    platformer_set_pos(&b, &controller, 34, 13 * 16 - 30);
    for (i = 0; i < 3; i++) { platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); }
    platformer_update(&b, &PHYS, &controller, 0, 1, 1); platformer_move(&b, &PHYS);
    for (i = 0; i < 10; i++) { platformer_update(&b, &PHYS, &controller, 0, 1, 0); platformer_move(&b, &PHYS); }
    CHECK(body_py(&b) < 13 * 16 - 30 - 20, "jump height y %d", body_py(&b));
    for (i = 0; i < 80; i++) { platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); }
    CHECK(body_py(&b) == 13 * 16 - 30 && (b.state & BODY_ON_GROUND), "landed y %d", body_py(&b));

    // ladder: climb from the floor to the ladder top, stand on it, climb back down
    platformer_set_pos(&b, &controller, 11 * 16 + 2, 13 * 16 - 30);
    for (i = 0; i < 300; i++) { platformer_ladder(&b, 1, 0, 0, 16); platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); }
    for (i = 0; i < 30; i++) { platformer_ladder(&b, 0, 0, 0, 16); platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); }
    CHECK(body_py(&b) == 8 * 16 - 30 && (b.state & BODY_ON_GROUND), "standing on ladder top y %d state %x", body_py(&b), b.state);
    for (i = 0; i < 300; i++) { platformer_ladder(&b, 0, 1, 0, 16); platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); }
    CHECK(body_py(&b) == 13 * 16 - 30, "back down y %d", body_py(&b));

    // one-way: drop onto it from above
    platformer_set_pos(&b, &controller, 4 * 16 + 2, 6 * 16);
    for (i = 0; i < 60; i++) { platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); }
    CHECK(body_py(&b) == 10 * 16 - 30 && (b.state & BODY_ON_GROUND), "landed on one-way y %d", body_py(&b));

    // one-way: jump up through it from below (high-jump tuning), land on top
    {
        static const platformer_config_t HIGH = { 4, 96, -100, -24, 4, 6, 28, 5, 6, 0 };
        platformer_set_pos(&b, &controller, 4 * 16 + 2, 13 * 16 - 30);
        for (i = 0; i < 3; i++) { platformer_update(&b, &HIGH, &controller, 0, 0, 0); platformer_move(&b, &HIGH); }
        platformer_update(&b, &HIGH, &controller, 0, 1, 1); platformer_move(&b, &HIGH);
        for (i = 0; i < 120; i++) { platformer_update(&b, &HIGH, &controller, 0, 1, 0); platformer_move(&b, &HIGH); }
        CHECK(body_py(&b) == 10 * 16 - 30, "jumped through one-way, y %d", body_py(&b));
    }

    // solid ceiling stops a jump
    platformer_set_pos(&b, &controller, 8 * 16 + 2, 13 * 16 - 30);
    for (i = 0; i < 3; i++) { platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); }
    {
        uint8_t hit = 0;
        platformer_update(&b, &PHYS, &controller, 0, 1, 1); platformer_move(&b, &PHYS); hit |= b.state;
        for (i = 0; i < 20; i++) { platformer_update(&b, &PHYS, &controller, 0, 1, 0); platformer_move(&b, &PHYS); hit |= b.state; }
        CHECK(hit & BODY_HIT_CEIL, "hit the solid ceiling");
    }

    // starting above the map with open outside (Bubble Keep wrap): falls in and lands on the top wall
    tileflags_set_outside(TF_NONE);
    platformer_set_pos(&b, &controller, 16 * 16, -60);
    b.vx = 0; b.vy = 0;
    for (i = 0; i < 200; i++) { platformer_update(&b, &PHYS, &controller, 0, 0, 0); platformer_move(&b, &PHYS); }
    CHECK(body_py(&b) == -30 && (b.state & BODY_ON_GROUND), "fell in from above the map, y %d", body_py(&b));
    // outside solid (default): left of the map blocks
    tileflags_set_outside(TF_SOLID);

    printf(fails ? "%d FAILED\n" : "all passed\n", fails);
    return fails != 0;
}

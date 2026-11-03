#include <zgdk/physics/platformer.h>
TILEFLAGS_DEFINE_CACHE(15);
body_t hero;
platformer_state_t control;
static const platformer_config_t config = {4,96,-70,-24,4,6,28,5,6,1};
void main(void) {
    platformer_init(&hero, &control, 32, 64, 12, 30);
    platformer_update(&hero, &config, &control, 1, 1, 1);
    platformer_move(&hero, &config);
}

#include <zgdk/physics/tilemap.h>
TILEFLAGS_DEFINE_CACHE(15);
body_t shot;
void main(void) {
    body_init(&shot, 32, 64, 4, 4);
    body_set_touch_mask(0);
    body_step(&shot);
}

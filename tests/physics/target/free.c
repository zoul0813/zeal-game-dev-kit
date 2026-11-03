#include <zgdk/physics/body.h>
_Static_assert(sizeof(body_t) == 12, "generic body layout");
body_t ship;
void main(void) {
    body_init(&ship, 32, 64, 12, 12);
    body_accelerate(&ship, 1, -1);
    body_integrate(&ship);
}

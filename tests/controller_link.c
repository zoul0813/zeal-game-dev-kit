/* One source, two link probes: pad-only and full mouse support. */
#include <zgdk/input/controller.h>
volatile uint16_t result;
void main(void) {
    controller_init();
    controller_flush();
    result = controller_read();
    result |= controller_get(SNES_PORT2);
    result |= controller_is(SNES_PORT1, SNES_PAD);
#ifdef TEST_MOUSE
    result |= controller_read_mouse(SNES_PORT1);
    result |= controller_is(SNES_PORT1, SNES_MOUSE);
    result |= controller_get_mouse_sensitivity(SNES_PORT1);
    result |= controller_set_mouse_sensitivity(SNES_PORT1, MOUSE_MEDIUM);
#endif
}

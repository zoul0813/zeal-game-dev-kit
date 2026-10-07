/* Compile this against the public header to exercise SDCC's real C ABI. */
#include <zgdk/input/controller.h>

volatile uint8_t test_port, test_target, test_byte;
volatile ControllerType test_kind;
volatile uint16_t test_word;
volatile int8_t test_x, test_y;

void test_init(void) { test_byte = controller_init(); }
void test_flush(void) { test_byte = controller_flush(); }
void test_read(void) { test_word = controller_read_port(test_port); }
void test_get(void) { test_word = controller_get(test_port); }
void test_mouse(void) {
    test_byte = controller_read_mouse(test_port);
    test_x = controller_get_mousex();
    test_y = controller_get_mousey();
}
/* Parentheses deliberately exercise the original externally callable symbol. */
void test_is(void) { test_byte = (controller_is)(test_port, test_kind); }
void test_pad(void) { test_byte = controller_is(test_port++, SNES_PAD); }
void test_dispatch(void) { test_byte = controller_is(test_port, test_kind); }
void test_sensitivity(void) { test_byte = controller_get_mouse_sensitivity(test_port); }
void test_set(void) {
    test_byte = controller_set_mouse_sensitivity(test_port, (MouseSensitivity)test_target);
}

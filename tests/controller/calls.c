/* C callers exercise argument/return conventions rather than guessing the ABI. */
#include <zgdk/input/controller.h>
typedef char pads_size_check[sizeof(ControllerPads)==4 ? 1 : -1];
typedef char mouse_size_check[sizeof(ControllerMouse)==4 ? 1 : -1];
uint16_t test_pad1(void) { return controller_read_pad(SNES_PORT1); }
uint16_t test_pad2(void) { return controller_read_pad(SNES_PORT2); }
void test_pads(void) { controller_read_pads((ControllerPads *)0x9000); }
uint8_t test_mouse1(void) { return controller_read_mouse(SNES_PORT1,(ControllerMouse *)0x9000); }
uint8_t test_mouse2(void) { return controller_read_mouse(SNES_PORT2,(ControllerMouse *)0x9000); }
uint8_t test_set1(void) { return controller_set_mouse_sensitivity(SNES_PORT1,MOUSE_HIGH); }
uint8_t test_set2(void) { return controller_set_mouse_sensitivity(SNES_PORT2,MOUSE_MEDIUM); }

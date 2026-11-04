#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H
#include <stdint.h>
extern uint16_t test_scroll_x, test_scroll_y;
extern uint8_t test_scroll_layer;
extern unsigned test_scroll_count, test_wait_count, test_upload_count, test_place_count;
extern uint8_t test_uploaded[512];
extern uint16_t test_uploaded_size, test_uploaded_from;
extern uint8_t test_placed_tile[2], test_placed_x[2], test_placed_y[2], test_placed_layer[2];
extern uint8_t test_graphics_error;
void test_reset_graphics(void);
#endif

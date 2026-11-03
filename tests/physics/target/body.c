/* Verify archive extraction for each free-body entry point. */
#include <zgdk/physics/body.h>

body_t first, second;

void main(void) {
#if defined(TEST_INIT)
    body_init(&first, 0, 0, 16, 16);
#elif defined(TEST_SET_POS)
    body_set_pos(&first, 0, 0);
#elif defined(TEST_OVERLAP)
    body_overlap(&first, &second);
#endif
}

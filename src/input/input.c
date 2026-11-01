#include "zgdk/input.h"

#define PAD_BUTTONS 0x0FFF

static uint8_t controller_mode = 0;
static uint16_t controller_suppressed = 0;

zos_err_t input_init(uint8_t use_controller)
{
    controller_mode = 0;
    controller_suppressed = 0;

    // init and flush the keyboard
    zos_err_t err = keyboard_init();
    if (err != ERR_SUCCESS)
        return err;

    err = keyboard_flush();
    if (err != ERR_SUCCESS)
        return -err;

    if (!use_controller)
        return ERR_SUCCESS;

    // init the controller
    err = controller_init();
    if (err != ERR_SUCCESS) {
        return ERR_BAD_MODE;
    }

    /* Require a release before accepting buttons held at startup. */
    controller_suppressed = PAD_BUTTONS;
    controller_mode = 1;

    return ERR_SUCCESS;
}

uint16_t input_read(void)
{
    uint16_t i = keyboard_read();
    if (controller_mode) {
        uint16_t pad = controller_read();
        if ((pad & 0xF000) == 0) {
            /* Only a valid pad report can release suppressed buttons. */
            controller_suppressed &= pad;
            i |= pad & ~controller_suppressed;
        } else {
            controller_suppressed = PAD_BUTTONS;
        }
    }

    return i;
}

void input_flush(void)
{
    keyboard_flush();
    if (controller_mode) {
        uint16_t pad = controller_read();
        if ((pad & 0xF000) == 0) {
            controller_suppressed = pad;
        } else {
            controller_suppressed = PAD_BUTTONS;
        }
    }
}

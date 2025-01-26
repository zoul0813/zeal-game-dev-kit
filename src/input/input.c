#include "input.h"

static uint8_t controller_mode = 0;

zos_err_t input_init(uint8_t use_controller) {
    // init and flush the keyboard
    zos_err_t err = keyboard_init();
    if(err != ERR_SUCCESS) return err;

    err = keyboard_flush();
    if(err != ERR_SUCCESS) return -err;

    if(!use_controller)
        return ERR_SUCCESS;

    // assume success, and enable controller
    controller_mode = 1;

    // init and test the controller
    err = controller_init();
    if(err != ERR_SUCCESS) {
        controller_mode = 0;
        return ERR_BAD_MODE;
    }

    err = controller_flush();
    if(err != ERR_SUCCESS) {
        controller_mode = 0;
        return ERR_ENTRY_CORRUPTED;
    }

    uint16_t test = controller_read();
    // if unconnected, we'll get back 0xFFFF (all buttons pressed)
    if (test & 0xFFFF) {
        controller_mode = 0;
    }

    return ERR_SUCCESS;
}

uint16_t input_get(void) {
    uint16_t i = keyboard_read();
    if (controller_mode == 1) {
        i |= controller_read();
    }

    return i;
}

void input_flush(void) {
    keyboard_flush();
    if(controller_mode) {
        controller_flush();
    }
}
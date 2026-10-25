#include <core.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_keyboard.h>
#include "zgdk/input/keyboard.h"
#include "zgdk/input/button_map.h"
#include "zgdk/utils/log.h"

uint16_t KEYBOARD_keys = 0;

#define KB_BUFFER_SIZE 32
/* Preserve the 32-byte alignment of the keyboard read buffer. */
static uint8_t array[KB_BUFFER_SIZE + (KB_BUFFER_SIZE - 1)];
static uint8_t* key_buffer;

static const struct {
    uint8_t code;
    uint16_t button;
} keys[] = {
    {KB_UP_ARROW, BUTTON_UP},
    {KB_DOWN_ARROW, BUTTON_DOWN},
    {KB_LEFT_ARROW, BUTTON_LEFT},
    {KB_RIGHT_ARROW, BUTTON_RIGHT},
    {KB_KEY_ENTER, BUTTON_START},
    {KB_RIGHT_SHIFT, BUTTON_SELECT},
    {KB_KEY_QUOTE, BUTTON_SELECT},
    {KB_KEY_Z, BUTTON_B},
    {KB_KEY_SPACE, BUTTON_B},
    {KB_KEY_X, BUTTON_A},
    {KB_KEY_A, BUTTON_Y},
    {KB_KEY_S, BUTTON_X},
    {KB_KEY_Q, BUTTON_L},
    {KB_KEY_W, BUTTON_R}
};
#define KEY_COUNT (sizeof(keys) / sizeof(keys[0]))

static uint8_t down[KEY_COUNT];
static uint8_t released;
/* Most recent horizontal and vertical presses, one direction bit per axis. */
static uint8_t preferred_directions;

static void keyboard_clear(void)
{
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
        down[i] = 0;
    }
    KEYBOARD_keys = 0;
    preferred_directions = 0;
}

static void keyboard_byte(uint8_t code)
{
    if (code == KB_RELEASED) {
        released = 1;
        return;
    }

    for (uint8_t i = 0; i < KEY_COUNT; i++) {
        if (keys[i].code == code) {
            uint16_t held = 0;
            uint16_t button = keys[i].button;
            /* Typematic repeats must not take priority from a newer press. */
            if (!released && !down[i]) {
                if (button & (BUTTON_LEFT | BUTTON_RIGHT)) {
                    preferred_directions =
                        (preferred_directions & ~(BUTTON_LEFT | BUTTON_RIGHT)) | button;
                }
                if (button & (BUTTON_UP | BUTTON_DOWN)) {
                    preferred_directions =
                        (preferred_directions & ~(BUTTON_UP | BUTTON_DOWN)) | button;
                }
            }
            down[i] = !released;
            for (uint8_t j = 0; j < KEY_COUNT; j++) {
                if (down[j]) {
                    held |= keys[j].button;
                }
            }
            if ((held & (BUTTON_LEFT | BUTTON_RIGHT)) == (BUTTON_LEFT | BUTTON_RIGHT)) {
                held = (held & ~(BUTTON_LEFT | BUTTON_RIGHT))
                     | (preferred_directions & (BUTTON_LEFT | BUTTON_RIGHT));
            }
            if ((held & (BUTTON_UP | BUTTON_DOWN)) == (BUTTON_UP | BUTTON_DOWN)) {
                held = (held & ~(BUTTON_UP | BUTTON_DOWN))
                     | (preferred_directions & (BUTTON_UP | BUTTON_DOWN));
            }
            KEYBOARD_keys = held;
            break;
        }
    }
    /* Unknown keys also consume a release prefix. */
    released = 0;
}

static zos_err_t keyboard_drain(uint8_t until_empty)
{
    while (1) {
        uint16_t size = KB_BUFFER_SIZE;
        zos_err_t err = read(DEV_STDIN, key_buffer, &size);
        if (err != ERR_SUCCESS) {
            return err;
        }
        if (size == 0) {
            return ERR_SUCCESS;
        }
        for (uint8_t i = 0; i < size; i++) {
            keyboard_byte(key_buffer[i]);
        }
        if (!until_empty && size < KB_BUFFER_SIZE) {
            return ERR_SUCCESS;
        }
    }
}

zos_err_t keyboard_flush(void)
{
    zos_err_t err = keyboard_drain(1);
    keyboard_clear();
    /* Retain a trailing release prefix so the next read stays in sync. */
    if (err != ERR_SUCCESS) {
        released = 0;
    }
    return err;
}

zos_err_t keyboard_init(void)
{
    key_buffer = (uint8_t*)((uintptr_t)(array + KB_BUFFER_SIZE - 1)
                           & (~(uintptr_t)(KB_BUFFER_SIZE - 1)));
    keyboard_clear();
    released = 0;
    return ioctl(DEV_STDIN, KB_CMD_SET_MODE, (void*)(KB_READ_NON_BLOCK | KB_MODE_RAW));
}

uint16_t keyboard_read(void)
{
    /* A short read ends this poll; later arrivals wait for the next poll. */
    zos_err_t err = keyboard_drain(0);
    if (err != ERR_SUCCESS) {
        log_error("Failed to read DEV_STDIN, clearing keys: ", err);
        keyboard_clear();
        released = 0;
    }
    return KEYBOARD_keys;
}

#include <stdio.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_keyboard.h>
#include "zgdk/input/keyboard.h"
#include "zgdk/input/button_map.h"

uint16_t KEYBOARD_keys = 0;

// DIRTY: trick to align key_buffer
#define KB_BUFFER_SIZE  32
static uint8_t array[KB_BUFFER_SIZE + (KB_BUFFER_SIZE - 1)];
static uint8_t* key_buffer;

/**
 * Flush the keyboard buffer
 */
zos_err_t keyboard_flush(void) {
  /* Flush the keyboard fifo */
  KEYBOARD_keys = 0;
  uint16_t size = sizeof(key_buffer);
  while (size) {
    zos_err_t err = read(DEV_STDIN, key_buffer, &size);
    if(err != ERR_SUCCESS) return err;
  }
  return ERR_SUCCESS;
}

/**
 * Initializes the keyboard
 */
zos_err_t keyboard_init(void) {
  zos_err_t err = ERR_FAILURE;
  /* Initialize the keyboard by setting it to raw and non-blocking */
  void* arg = (void*) (KB_READ_NON_BLOCK | KB_MODE_RAW);
  err = ioctl(DEV_STDIN, KB_CMD_SET_MODE, arg);

  // DIRTY: trick to align key_buffer
  key_buffer = (uint8_t*)((uintptr_t)(array + 31) & (~(uintptr_t)31));

  return err;
}

uint16_t keyboard_read(void) {
  uint16_t size = KB_BUFFER_SIZE;
  while(1) {
    zos_err_t err = read(DEV_STDIN, key_buffer, &size);
    if(err != ERR_SUCCESS) {
      printf("Failed to read DEV_STDIN, clearing keys: %d", err);
      KEYBOARD_keys = 0;
      return KEYBOARD_keys;
    }

    if (size == 0) {
      return KEYBOARD_keys;
    }

    uint8_t released = 0;
    for (uint8_t i = 0; i < size; i++) {
      if (key_buffer[i] == KB_RELEASED) {
        released = 1;
      } else {
        switch(key_buffer[i]) {
          case KB_KEY_W:
          case KB_UP_ARROW:
            if(released) {
              KEYBOARD_keys &= ~BUTTON_UP;
            } else {
              // KEYBOARD_keys |= BUTTON_UP;
              KEYBOARD_keys = (KEYBOARD_keys & ~BUTTON_DOWN) | BUTTON_UP;

            }
            break;

          case KB_KEY_S:
          case KB_DOWN_ARROW:
            if(released) {
              KEYBOARD_keys &= ~BUTTON_DOWN;
            } else {
              // KEYBOARD_keys |= BUTTON_DOWN;
              KEYBOARD_keys = (KEYBOARD_keys & ~BUTTON_UP) | BUTTON_DOWN;
            }
            break;

          case KB_KEY_A:
          case KB_LEFT_ARROW:
            if(released) {
              KEYBOARD_keys &= ~BUTTON_LEFT;
            } else {
              // KEYBOARD_keys |= BUTTON_LEFT;
              KEYBOARD_keys = (KEYBOARD_keys & ~BUTTON_RIGHT) | BUTTON_LEFT;
            }
            break;

          case KB_KEY_D:
          case KB_RIGHT_ARROW:
            if(released) {
              KEYBOARD_keys &= ~BUTTON_RIGHT;
            } else {
              KEYBOARD_keys = (KEYBOARD_keys & ~BUTTON_LEFT) | BUTTON_RIGHT;

            }
            break;
          case KB_KEY_ENTER:
            if(released) {
              KEYBOARD_keys &= ~BUTTON_START;
            } else {
              KEYBOARD_keys |= BUTTON_START;
            }
            break;
          case KB_KEY_QUOTE:
            if(released) {
              KEYBOARD_keys &= ~BUTTON_SELECT;
            } else {
              KEYBOARD_keys |= BUTTON_SELECT;
            }
            break;
          case KB_KEY_SPACE:
            if(released) {
              KEYBOARD_keys &= ~BUTTON_B;
            } else {
              KEYBOARD_keys |= BUTTON_B;
            }
            break;
        }
        released = 0;
      }
    }
  }
}
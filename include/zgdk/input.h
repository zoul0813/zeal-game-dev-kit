#include <zos_errors.h>
#include "zgdk/input/button_map.h"
#include "zgdk/input/controller.h"
#include "zgdk/input/keyboard.h"

/**
 * @brief Initialize input
 * Pad buttons must be seen released before their first activation.
 *
 * @param use_controller Attempt to use a controller?
 */
zos_err_t input_init(uint8_t use_controller);

/**
 * @brief Get keyboard and valid SNES pad input (port 1).
 * Invalid reports require pad buttons to be seen released before activating again.
 *
 * @return 16-bit mask of ZGDK Inputs
 */
uint16_t input_read(void);

/** @deprecated */
#define input_get() input_read()

/**
 * @brief Drain and clear keyboard input; suppress held pad buttons until release.
 *
 * Suppression applies to input_read(), not raw controller reads. Each pad
 * button becomes available again after a valid report shows it released.
 */
void input_flush(void);

#include <zos_errors.h>
#include "zgdk/input/button_map.h"
#include "zgdk/input/controller.h"
#include "zgdk/input/keyboard.h"

/**
 * @brief Initialize input
 *
 * @param use_controller Attempt to use a controller?
 */
zos_err_t input_init(uint8_t use_controller);

/**
 * @brief Get user input
 *
 * @return 16-bit mask of ZGDK Inputs
 */
uint16_t input_get(void);

/**
 * @brief flush inputs to zero them out
 */
void input_flush(void);

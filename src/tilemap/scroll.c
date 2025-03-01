#include <zvb_gfx.h>
#include "sdcc.h"
#include "zgdk/tilemap/scroll.h"

/**
 * @brief set the current tilemap `x` offset on `layer`
 */
void tilemap_scroll_x(uint8_t layer, uint16_t value)
{
    if (layer == 0) {
        zvb_ctrl_l0_scr_x_low  = value & 0xFF;
        zvb_ctrl_l0_scr_x_high = value >> 8;
    } else {
        zvb_ctrl_l1_scr_x_low  = value & 0xFF;
        zvb_ctrl_l1_scr_x_high = value >> 8;
    }
}

/**
 * @brief set the current tilemap `y` offset on `layer`
 */
void tilemap_scroll_y(uint8_t layer, uint16_t value)
{
    if (layer == 0) {
        zvb_ctrl_l0_scr_y_low  = value & 0xFF;
        zvb_ctrl_l0_scr_y_high = value >> 8;
    } else {
        zvb_ctrl_l1_scr_y_low  = value & 0xFF;
        zvb_ctrl_l1_scr_y_high = value >> 8;
    }
}

/**
 * @brief set the current tilemap `x` and `y` offset on `layer`
 */
void tilemap_scroll(uint8_t layer, uint16_t x, uint16_t y)
{
    tilemap_scroll_x(layer, x);
    tilemap_scroll_y(layer, y);
}

/**
 * @brief get the current tilemap `layer` x offset
 */
uint16_t tilemap_scroll_get_x(uint8_t layer)
{
    uint16_t value;
    if (layer == 0) {
        value  = zvb_ctrl_l0_scr_x_high << 8;
        value |= zvb_ctrl_l0_scr_x_low;
    } else {
        value  = zvb_ctrl_l1_scr_x_high << 8;
        value |= zvb_ctrl_l1_scr_x_low;
    }
    return value;
}

/**
 * @brief get the current tilemap `layer` y offset
 */
uint16_t tilemap_scroll_get_y(uint8_t layer)
{
    uint16_t value;
    if (layer == 0) {
        value  = zvb_ctrl_l0_scr_y_high << 8;
        value |= zvb_ctrl_l0_scr_y_low;
    } else {
        value  = zvb_ctrl_l1_scr_y_high << 8;
        value |= zvb_ctrl_l1_scr_y_low;
    }
    return value;
}

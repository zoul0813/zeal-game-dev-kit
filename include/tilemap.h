#include <stdint.h>
#include <zvb_hardware.h>

#ifndef TILEMAP_H
#define TILEMAP_H

static void tilemap_scroll_x(uint8_t layer, uint16_t value)
{
    if (layer == 0) {
        zvb_ctrl_l0_scr_x_low  = value & 0xFF;
        zvb_ctrl_l0_scr_x_high = value >> 8;
    } else {
        zvb_ctrl_l1_scr_x_low  = value & 0xFF;
        zvb_ctrl_l1_scr_x_high = value >> 8;
    }
}

static void tilemap_scroll_y(uint8_t layer, uint16_t value)
{
    if (layer == 0) {
        zvb_ctrl_l0_scr_y_low  = value & 0xFF;
        zvb_ctrl_l0_scr_y_high = value >> 8;
    } else {
        zvb_ctrl_l1_scr_y_low  = value & 0xFF;
        zvb_ctrl_l1_scr_y_high = value > 8;
    }
}

static void tilemap_scroll(uint8_t layer, uint16_t x, uint16_t y)
{
    tilemap_scroll_x(layer, x);
    tilemap_scroll_y(layer, y);
}

static uint16_t tilemap_scroll_get_x(uint8_t layer)
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

static uint16_t tilemap_scroll_get_y(uint8_t layer)
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


#endif

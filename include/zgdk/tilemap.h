#include "sdcc.h"

#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_hardware.h>


#ifndef TILEMAP_H
#define TILEMAP_H

/**
 * @brief set the current tilemap `x` offset on `layer`
 */
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

/**
 * @brief set the current tilemap `y` offset on `layer`
 */
static void tilemap_scroll_y(uint8_t layer, uint16_t value)
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
static void tilemap_scroll(uint8_t layer, uint16_t x, uint16_t y)
{
    tilemap_scroll_x(layer, x);
    tilemap_scroll_y(layer, y);
}

/**
 * @brief get the current tilemap `layer` x offset
 */
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

/**
 * @brief get the current tilemap `layer` y offset
 */
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

/**
 * @brief Fill the tilemap with `tile` from x,y to x+width,y+height with `tile` on `layer`
 */
static void tilemap_fill(gfx_context *ctx, uint8_t tile, uint8_t layer, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    for(uint8_t j = 0; j < height; j++) {
        uint8_t line[80];
        for(uint8_t i = 0; i < width; i++) {
            line[i] = tile;
        }
        gfx_tilemap_load(ctx, line, width, layer, x, y + j);
    }
}

#endif

#include <stdint.h>

#ifndef TILEMAP_SCROLL_H
#define TILEMAP_SCROLL_H

/**
 * @brief set the current tilemap `x` offset on `layer`
 */
void tilemap_scroll_x(uint8_t layer, uint16_t value);

/**
 * @brief set the current tilemap `y` offset on `layer`
 */
void tilemap_scroll_y(uint8_t layer, uint16_t value);

/**
 * @brief set the current tilemap `x` and `y` offset on `layer`
 */
void tilemap_scroll(uint8_t layer, uint16_t x, uint16_t y);

/**
 * @brief get the current tilemap `layer` x offset
 */
uint16_t tilemap_scroll_get_x(uint8_t layer);

/**
 * @brief get the current tilemap `layer` y offset
 */
uint16_t tilemap_scroll_get_y(uint8_t layer);

#endif
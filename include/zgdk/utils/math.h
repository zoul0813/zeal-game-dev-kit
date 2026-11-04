#ifndef ZGDK_UTILS_MATH_H
#define ZGDK_UTILS_MATH_H
#include <stdint.h>

/* Inclusive bounds; min must be <= max. One archive member per function. */
int16_t clamp_i16(int16_t value, int16_t min, int16_t max);
uint16_t clamp_u16(uint16_t value, uint16_t min, uint16_t max);
int8_t clamp_i8(int8_t value, int8_t min, int8_t max);
uint8_t clamp_u8(uint8_t value, uint8_t min, uint8_t max);

#endif

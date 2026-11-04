#include <zgdk/utils/math.h>

uint8_t clamp_u8(uint8_t value, uint8_t min, uint8_t max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

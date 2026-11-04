#include <zgdk/utils/math.h>

uint16_t clamp_u16(uint16_t value, uint16_t min, uint16_t max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

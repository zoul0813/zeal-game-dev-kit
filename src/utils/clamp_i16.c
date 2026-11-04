#include <zgdk/utils/math.h>

int16_t clamp_i16(int16_t value, int16_t min, int16_t max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

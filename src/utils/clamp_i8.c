#include <zgdk/utils/math.h>

int8_t clamp_i8(int8_t value, int8_t min, int8_t max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

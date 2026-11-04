#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <zgdk/utils/math.h>

int main(void) {
    int value, lo, hi;
    /* Exhaust every 8-bit value, including full signed/unsigned limits. */
    for (value = -128; value <= 127; ++value) {
        for (lo = -128; lo <= 127; lo += 17) for (hi = lo; hi <= 127; hi += 17) {
            int expected = value < lo ? lo : value > hi ? hi : value;
            assert(clamp_i8(value, lo, hi) == expected);
        }
        assert(clamp_i8(value, INT8_MIN, INT8_MAX) == value);
    }
    for (value = 0; value <= 255; ++value) {
        assert(clamp_u8(value, 10, 240) == (value < 10 ? 10 : value > 240 ? 240 : value));
        assert(clamp_u8(value, 0, UINT8_MAX) == value);
    }
    assert(clamp_i16(INT16_MIN, -100, 100) == -100);
    assert(clamp_i16(INT16_MAX, -100, 100) == 100);
    assert(clamp_i16(-100, -100, 100) == -100);
    assert(clamp_i16(100, -100, 100) == 100);
    assert(clamp_i16(12, -100, 100) == 12);
    assert(clamp_i16(INT16_MIN, INT16_MIN, INT16_MAX) == INT16_MIN);
    assert(clamp_i16(INT16_MAX, INT16_MIN, INT16_MAX) == INT16_MAX);
    assert(clamp_i16(12, -3, -3) == -3);
    assert(clamp_u16(0, 100, 60000) == 100);
    assert(clamp_u16(UINT16_MAX, 100, 60000) == 60000);
    assert(clamp_u16(12345, 100, 60000) == 12345);
    assert(clamp_u16(0, 0, UINT16_MAX) == 0);
    assert(clamp_u16(UINT16_MAX, 0, UINT16_MAX) == UINT16_MAX);
    assert(clamp_u16(12, 3, 3) == 3);
    puts("signed/unsigned integer clamps: passed");
    return 0;
}

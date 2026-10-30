#include "stream_internal.h"

/* Remainder for the 80-column or 40-row ring, without software division.
 * Halving multiples cover every uint16_t input in at most eleven compares.
 * offset is already in 0..period-1; their sum fits uint8_t. */
uint8_t zgdk_stream_ring_at(uint16_t tile, uint8_t offset, uint8_t period) {
    uint16_t step = 40960U;
    uint8_t r;
    do {
        if (tile >= step) tile -= step;
        step >>= 1;
    } while (step >= period);
    r = (uint8_t)tile + offset;
    if (r >= period) r -= period;
    return r;
}

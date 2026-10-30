#include <assert.h>
#include <stdio.h>
#include "stream_internal.h"

int main(void) {
    uint32_t tile;
    uint8_t period, offset;
    /* Independent remainder oracle, including large rebase offsets. */
    for (period = 40; period <= 80; period += 40)
        for (offset = 0; offset < period; offset++)
            for (tile = 0; tile <= UINT16_MAX; tile++)
                assert(zgdk_stream_ring_at((uint16_t)tile, offset, period) ==
                       (tile + offset) % period);
    puts("stream ring: every 16-bit coordinate and valid offset passed");
    return 0;
}

#include <assert.h>
#include <stdio.h>
#include "zgdk/sound/tracker.h"
_Static_assert(TICKS_PER_QUARTER(32) == 32, "quarter-note tempo");
_Static_assert(TICKS_PER_QUARTER(12 + 20) == 32, "expression argument");
int main(void)
{
    unsigned tempo = 28;
    assert(TICKS_PER_QUARTER(tempo) == 28);
    assert(2 * TICKS_PER_QUARTER(tempo + 4) == 64);
    assert(TICKS_PER_QUARTER(tempo++) == 28 && tempo == 29);
    puts("tracker header checks passed");
}

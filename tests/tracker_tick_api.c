/* Exercise the real SDCC ABI and headers for cycle comparisons. */
#include <string.h>
#include <zgdk/sound/tracker.h>
#include "../src/sound/tracker/private.h"

static pattern_t pattern;
static track_t track;
volatile uint8_t test_tempo, test_phase, test_next;
void test_tick(void)
{
    track.pattern_count = 1;
    track.patterns[0] = &pattern;
    track.current_tempo = test_tempo;
    zmt_ticks = test_phase;
    zmt_current_pattern = 0;
    zmt_current_arrangement = 0;
    zmt_next_step = 5;
    /* All fields empty: measure scheduling plus the same empty playback. */
    for (uint8_t v = 0; v < NUM_VOICES; v++) {
        step_t *step = &pattern.voices[v].steps[5];
        step->note = NOTE_OUT_OF_RANGE;
        step->waveform = WAVEFORM_OUT_OF_RANGE;
        step->fx1 = step->fx2 = FX_OUT_OF_RANGE;
    }
    test_next = zmt_tick(&track, 0);
}
/* Core memory shims for this standalone target; not part of the SDK library. */
void *mem_set(void *dst, uint8_t value, size_t size) { return memset(dst, value, size); }
void *mem_cpy(void *dst, const void *src, size_t size) { return memcpy(dst, src, size); }

#include "zgdk/sound/tracker.h"
#include "private.h"

uint8_t zmt_pattern_prev(track_t* track)
{
    if (current_pattern > 0)
        current_pattern--;
    else {
        current_pattern = track->pattern_count - 1;
    }
    return current_pattern;
}
/* recede to the next pattern */
uint8_t zmt_pattern_next(track_t* track)
{
    current_pattern++;
    if (current_pattern >= track->pattern_count)
        current_pattern = 0;
    return current_pattern;
}
/* set the current pattern */
uint8_t zmt_pattern_set(track_t* track, uint8_t index)
{
    current_pattern = index;
    if (current_pattern > track->pattern_count)
        current_pattern = 0;
    return current_pattern;
}

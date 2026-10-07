#include "zgdk/sound/tracker.h"
#include "private.h"

uint8_t zmt_pattern_prev(track_t* track)
{
    if (track == NULL || track->pattern_count == 0 || track->pattern_count > NUM_PATTERNS)
        return zmt_current_pattern = 0;
    if (zmt_current_pattern > 0 && zmt_current_pattern < track->pattern_count)
        zmt_current_pattern--;
    else {
        zmt_current_pattern = track->pattern_count - 1;
    }
    return zmt_current_pattern;
}
/* recede to the next pattern */
uint8_t zmt_pattern_next(track_t* track)
{
    if (track == NULL || track->pattern_count == 0 || track->pattern_count > NUM_PATTERNS)
        return zmt_current_pattern = 0;
    zmt_current_pattern++;
    if (zmt_current_pattern >= track->pattern_count)
        zmt_current_pattern = 0;
    return zmt_current_pattern;
}
/* set the current pattern */
uint8_t zmt_pattern_set(track_t* track, uint8_t index)
{
    zmt_current_pattern = 0;
    if (track != NULL && track->pattern_count > 0 && track->pattern_count <= NUM_PATTERNS
        && index < track->pattern_count)
        zmt_current_pattern = index;
    return zmt_current_pattern;
}

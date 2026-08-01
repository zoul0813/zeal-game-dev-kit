#include "zgdk/sound/tracker.h"
#include "private.h"

uint8_t zmt_track_get_arrangement(track_t* track)
{
    track; // unreferenced
    return zmt_current_arrangement;
}
uint8_t zmt_track_get_pattern(track_t* track)
{
    track; // unreferenced
    return zmt_current_pattern;
}
uint8_t zmt_track_get_next_step(track_t* track)
{
    track; // unreferenced
    return zmt_next_step;
}
uint8_t zmt_track_get_last_step(track_t* track)
{
    track; // unreferenced
    return zmt_last_step;
}
uint8_t zmt_track_get_frame(track_t* track)
{
    track; // unreferenced
    return zmt_ticks;
}

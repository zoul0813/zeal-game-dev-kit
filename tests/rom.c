#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "zgdk/sound/tracker.h"
#include "../src/sound/tracker/private.h"
static const uint8_t valid[] = {
    'Z','M','T',0, 'T','e','s','t',' ','t','i','t','l','e','!','!', 28,1,
    1,0,0,0,0,0,0,0, 0,0xFF,
    1, 1,0,0,0, 15, 48,2,FX_COUNT_2,FX_GOTO_10
};
int main(void)
{
    static pattern_t pattern, saved;
    track_t track = {0}, before;
    track.patterns[0] = &pattern;
    zmt_track_init(&track);
    before = track; saved = pattern;
    zmt_ticks = 7;
    for (unsigned n = 0; n < sizeof(valid); n++) {
        uint8_t *prefix = n ? malloc(n) : NULL;
        if (n) memcpy(prefix, valid, n);
        assert(zmt_rom_load(&track, n ? prefix : valid, n) != ERR_SUCCESS);
        assert(memcmp(&track, &before, sizeof(track)) == 0);
        assert(memcmp(&pattern, &saved, sizeof(pattern)) == 0 && zmt_ticks == 7);
        free(prefix);
    }
    assert(zmt_rom_load(NULL, valid, sizeof(valid)) == ERR_INVALID_PARAMETER);
    assert(zmt_rom_load(&track, NULL, 1) == ERR_INVALID_PARAMETER);
    const unsigned offsets[] = {0,3,17,28,33,34,35};
    const uint8_t values[] = {'X',1,17,16,16,109,4};
    for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); i++) {
        uint8_t bad[sizeof(valid)]; memcpy(bad, valid, sizeof(bad));
        bad[offsets[i]] = values[i];
        assert(zmt_rom_load(&track, bad, sizeof(bad)) != ERR_SUCCESS);
        assert(memcmp(&track, &before, sizeof(track)) == 0 && zmt_ticks == 7);
        assert(memcmp(&pattern, &saved, sizeof(pattern)) == 0);
    }
    uint8_t bad[sizeof(valid)]; memcpy(bad,valid,sizeof(bad));
    bad[17]=0; assert(zmt_rom_load(&track,bad,sizeof(bad))!=ERR_SUCCESS);
    bad[17]=1; bad[26]=1; assert(zmt_rom_load(&track,bad,sizeof(bad))!=ERR_SUCCESS);
    track.patterns[0]=NULL;
    assert(zmt_rom_load(&track,valid,sizeof(valid))==ERR_INVALID_PARAMETER);
    track.patterns[0]=&pattern;
    assert(zmt_rom_load(&track,valid,sizeof(valid))==ERR_SUCCESS);
    assert(strcmp(track.title,"Test title!!")==0 && track.tempo==28 && track.current_tempo==28);
    assert(zmt_ticks==0 && track.arrangement[0].pattern_index==0);
    assert(track.arrangement[1].pattern_index==ARRANGEMENT_OUT_OF_RANGE);
    step_t *step=&pattern.voices[0].steps[0];
    assert(step->note==48 && step->waveform==2 && step->fx1==FX_COUNT_2 && step->fx2==FX_GOTO_10);
    assert(step->fx1_attr==0 && pattern.voices[0].steps[1].note==NOTE_OUT_OF_RANGE);
    uint8_t *pos=(uint8_t *)valid+28;
    assert(zmt_pattern_rom_load(&pattern,&pos)==ERR_SUCCESS && pos==valid+sizeof(valid));
    pos=(uint8_t *)valid+18;
    assert(zmt_arrangement_rom_load(track.arrangement,&pos)==ERR_SUCCESS && pos==valid+28);
    uint8_t minimal[27]={ 'Z','M','T',0 };
    minimal[16]=32; minimal[17]=1;
    assert(zmt_rom_load(&track,minimal,sizeof(minimal))==ERR_SUCCESS);
    assert(track.arrangement[0].pattern_index==ARRANGEMENT_OUT_OF_RANGE);
    puts("tracker ROM checks passed");
}

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/sound/tracker.h"
extern uint8_t zmt_ticks,zmt_next_step,zmt_last_step,zmt_current_pattern,zmt_current_arrangement;
static pattern_t patterns[2];
static track_t track;
static void init(void){
    memset(&track,0,sizeof(track));track.patterns[0]=&patterns[0];track.patterns[1]=&patterns[1];
    zmt_track_init(&track);zmt_pattern_init(&patterns[1]);
}
int main(void){
    init();
    for(unsigned f=0;f<66;f++){
        uint8_t old=zmt_next_step;zmt_tick(&track,0);
        assert((zmt_next_step!=old)==(f%8==0));
    }
    init();track.current_tempo=9;
    for(unsigned f=0;f<24;f++){
        uint8_t old=zmt_next_step;zmt_tick(&track,0);
        assert((zmt_next_step!=old)==(f%2==0));
    }
    for(unsigned tempo=0;tempo<4;tempo++){
        init();track.current_tempo=tempo;assert(zmt_tick(&track,0)==0);
    }
    init();assert(zmt_pattern_set(&track,1)==0 && zmt_pattern_set(&track,255)==0);
    track.pattern_count=0;assert(zmt_pattern_prev(&track)==0 && zmt_pattern_next(&track)==0);
    assert(zmt_tick(&track,1)==0 && zmt_pattern_set(NULL,1)==0);
    init();track.pattern_count=17;assert(zmt_tick(&track,1)==0);
    init();track.patterns[0]=NULL;assert(zmt_tick(&track,1)==0);
    init();for(unsigned a=0;a<64;a++)track.arrangement[a].pattern_index=0;
    zmt_current_arrangement=63;zmt_next_step=31;zmt_tick(&track,1);
    assert(zmt_current_arrangement==0 && zmt_current_pattern==0 && zmt_next_step==0);
    init();track.pattern_count=2;track.arrangement[1].pattern_index=0;track.arrangement[1].fx=3;
    track.arrangement[3].pattern_index=1;zmt_next_step=31;zmt_tick(&track,1);
    assert(zmt_current_arrangement==3 && zmt_current_pattern==1);
    init();track.arrangement[1].pattern_index=16;zmt_next_step=31;zmt_tick(&track,1);
    assert(zmt_tick(&track,1)==0); /* Invalid arrangement pattern cannot be dereferenced. */
    for(unsigned target=0;target<32;target++){
        init();patterns[0].voices[0].steps[0].fx1=FX_GOTO_0+target;
        zmt_tick(&track,0);assert(zmt_next_step==target && zmt_ticks==1);
    }
    init();step_t s;zmt_step_init(&s,0);s.note=127;
    zvb_peri_sound_freq_low=77;zmt_play_step(&s,VOICE0);assert(zvb_peri_sound_freq_low==77);
    zmt_play_pattern(&patterns[0],32);zmt_play_pattern(NULL,0);
    zmt_ticks=31;zmt_next_step=22;zmt_current_arrangement=63;zmt_track_init(&track);
    assert(zmt_ticks==0 && zmt_next_step==0 && zmt_current_arrangement==0);
    puts("tracker playback checks passed");
}

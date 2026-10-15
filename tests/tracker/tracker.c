#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

uint8_t zvb_peri_sound_freq_low, zvb_peri_sound_freq_high;
uint8_t zvb_peri_sound_wave, zvb_peri_sound_select, zvb_peri_sound_volume;
uint8_t zvb_peri_sound_master_vol, zvb_peri_sound_hold, zvb_config_dev_idx;
uint8_t zvb_peri_sound_left_channel, zvb_peri_sound_right_channel;
void* tracker_test_memcpy(void* dst, const void* src, size_t size)
{
    return memcpy(dst,src,size);
}
static track_t track;
static pattern_t patterns[NUM_PATTERNS];

static void init(void)
{
    memset(&track, 0, sizeof track);
    for (uint8_t p=0; p<NUM_PATTERNS; p++) {
        track.patterns[p]=&patterns[p];
        zmt_pattern_init(&patterns[p]);
    }
    zmt_track_init(&track);
    zmt_track_reset(&track, 1);
}

static void test_reset(void)
{
    zvb_peri_sound_wave=0;
    zvb_peri_sound_left_channel=0;
    zvb_peri_sound_right_channel=0;
    zvb_config_dev_idx=5;
    zmt_reset(VOL_75);
    assert(zvb_peri_sound_left_channel==VOICEALL);
    assert(zvb_peri_sound_right_channel==VOICEALL);
    assert(zvb_peri_sound_wave==(WAV_SQUARE | DUTY_CYCLE_50_0));
    assert(zvb_config_dev_idx==5);
}

static void test_master_volume(void)
{
    const uint8_t volumes[]={VOL_0,VOL_25,VOL_50,VOL_75,VOL_100};
    const uint8_t encoded[]={0xC0,0,5,10,15};
    for (uint8_t i=0;i<5;i++) {
        zmt_reset(volumes[i]);
        assert(zvb_peri_sound_master_vol==encoded[i]);
    }
}

static void test_note_mute(void)
{
    step_t step;
    zmt_step_init(&step,0);
    zmt_reset(VOL_75);
    step.note=NOTE_MIDDLE_C;
    step.waveform=WAV_NOISE;
    zmt_play_step(&step,VOICE0);
    assert(zvb_peri_sound_volume==VOL_75);
    step.note=NOTE_OFF;
    zmt_play_step(&step,VOICE0);
    assert(zvb_peri_sound_volume==VOL_0);
    step.note=NOTE_MIDDLE_C;
    zmt_play_step(&step,VOICE0);
    assert(zvb_peri_sound_volume==VOL_75);
    zmt_sound_off();
    assert(zvb_peri_sound_select==VOICEALL);
    assert(zvb_peri_sound_volume==VOL_0);
    step.note=NOTE_OUT_OF_RANGE;
    zmt_play_step(&step,VOICE0);
    assert(zvb_peri_sound_volume==VOL_0);
    step.note=NOTE_MIDDLE_C;
    zmt_play_step(&step,VOICE0);
    assert(zvb_peri_sound_volume==VOL_75);
}

static void test_voice_volume(void)
{
    step_t step;
    zmt_step_init(&step,0);
    zmt_reset(VOL_75);
    uint8_t master=zvb_peri_sound_master_vol;
    step.note=NOTE_MIDDLE_C;
    step.fx1=FX_VOL_50_0;
    zmt_play_step(&step,VOICE0);
    assert(zvb_peri_sound_select==VOICE0);
    assert(zvb_peri_sound_volume==VOL_50);
    assert(zvb_peri_sound_master_vol==master);
    step.fx1=FX_OUT_OF_RANGE;
    zmt_play_step(&step,VOICE1);
    assert(zvb_peri_sound_volume==VOL_75);
    step.note=NOTE_OFF;
    step.fx2=FX_VOL_100;
    zmt_play_step(&step,VOICE0);
    assert(zvb_peri_sound_volume==VOL_0);
    step.note=NOTE_MIDDLE_C;
    step.fx2=FX_OUT_OF_RANGE;
    zmt_play_step(&step,VOICE0);
    assert(zvb_peri_sound_volume==VOL_100);
}

static void test_tempo(void)
{
    for (uint8_t tempo=4;tempo<=128;tempo+=4) {
        init();
        track.tempo=track.current_tempo=tempo;
        uint16_t previous=0;
        uint16_t count=0;
        for (uint16_t frame=0;frame<(uint16_t)tempo*4;frame++) {
            uint8_t next=zmt_next_step;
            zmt_tick(&track,0);
            if (next!=zmt_next_step) {
                if(count) assert(frame-previous==tempo/4);
                previous=frame;
                count++;
            }
        }
        assert(count==16);
    }
}

static void test_arrangement_bounds(void)
{
    init();
    for(uint8_t i=0;i<NUM_ARRANGEMENTS;i++)
        track.arrangement[i].pattern_index=0;
    zmt_current_arrangement=63;
    zmt_next_step=31;
    zmt_tick(&track,1);
    assert(zmt_current_arrangement==0 && zmt_current_pattern==0);
    track.arrangement[0].pattern_index=ARRANGEMENT_OUT_OF_RANGE;
    track.arrangement[1].pattern_index=99;
    zmt_next_step=31;
    zmt_ticks=0;
    zmt_tick(&track,1);
    assert(zmt_current_arrangement==0 && zmt_current_pattern==0);
    zmt_ticks=0;
    zmt_tick(&track,1);
    assert(zmt_current_pattern==0);
    assert(zmt_track_reset(&track,1)==0);
}

static void test_arrangement_goto(void)
{
    init();
    track.pattern_count=2;
    track.arrangement[1]=(arrangement_t){1,0};
    zmt_next_step=31;
    zmt_tick(&track,1);
    assert(zmt_current_arrangement==0 && zmt_current_pattern==0);
    track.arrangement[0].fx=1;
    track.arrangement[1].fx=FX_OUT_OF_RANGE;
    zmt_track_reset(&track,1);
    assert(zmt_current_arrangement==1 && zmt_current_pattern==1);
    track.arrangement[0].fx=0x20;
    zmt_track_reset(&track,1);
    assert(track.current_tempo==4);
    assert(zmt_current_arrangement==0 && zmt_current_pattern==0);
}

static void test_pattern_goto_timing(void)
{
    const uint8_t tempos[]={4,16,32,128};
    for(uint8_t t=0;t<4;t++) for(uint8_t target=0;target<32;target++) {
        init();
        track.current_tempo=tempos[t];
        patterns[0].voices[0].steps[7].fx2=FX_GOTO_0+target;
        zmt_next_step=7;
        zmt_tick(&track,0);
        assert(zmt_next_step==target);
        for(uint8_t frame=1;frame<=tempos[t]/4;frame++) {
            zmt_last_step=255;
            zmt_tick(&track,0);
            assert(zmt_last_step==(frame==tempos[t]/4 ? target : 255));
        }
    }
}

static void test_countdown_restart(void)
{
    init();
    track.pattern_count=2;
    step_t* step=&patterns[1].voices[0].steps[0];
    step->fx1=FX_COUNT_1;
    step->fx2=FX_VOICE_SAW;
    zmt_reset(VOL_75);
    zmt_play_step(step,VOICE0);
    assert(step->fx1_attr==1);
    assert(zvb_peri_sound_wave==(WAV_SQUARE | DUTY_CYCLE_50_0));
    zmt_track_reset(&track,1);
    assert(step->fx1_attr==0);
    zmt_play_step(step,VOICE0);
    assert(step->fx1_attr==1);
    assert(zvb_peri_sound_wave==(WAV_SQUARE | DUTY_CYCLE_50_0));
    patterns[0].voices[3].steps[31].fx2_attr=9;
    zmt_track_reset(&track,0);
    assert(patterns[0].voices[3].steps[31].fx2_attr==0);
    assert(step->fx1_attr==0);
}

int main(void)
{
    init();
    test_reset();
    test_master_volume();
    test_note_mute();
    test_voice_volume();
    test_tempo();
    test_arrangement_bounds();
    test_arrangement_goto();
    test_pattern_goto_timing();
    test_countdown_restart();
    puts("tracker tests passed");
    return 0;
}

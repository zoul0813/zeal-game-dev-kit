#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

uint8_t zvb_peri_sound_freq_low, zvb_peri_sound_freq_high;
uint8_t zvb_peri_sound_wave, zvb_peri_sound_select, zvb_peri_sound_volume;
uint8_t zvb_peri_sound_master_vol, zvb_peri_sound_hold, zvb_config_dev_idx;
uint8_t zvb_peri_sound_left_channel, zvb_peri_sound_right_channel;
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

int main(void)
{
    init();
    test_reset();
    test_master_volume();
    puts("tracker tests passed");
    return 0;
}

#include <assert.h>
#include <stdio.h>
#include "zgdk/sound/tracker.h"
int main(void){
    for(unsigned voice=VOICE0;voice<=VOICE3;voice<<=1){
        for(unsigned fx=FX_VOL_00_0;fx<=FX_VOL_100;fx++){
            step_t step;zmt_step_init(&step,0);
            step.note=NOTE_MIDDLE_C;step.waveform=1;step.fx1=fx;
            zmt_play_step(&step,voice);
            assert(zvb_peri_sound_select==voice);
            assert((zvb_peri_sound_freq_low | (zvb_peri_sound_freq_high<<8))==NOTES[NOTE_MIDDLE_C]);
            assert(zvb_peri_sound_wave==(1|DUTY_CYCLE_50_0));
            step.fx1=FX_OUT_OF_RANGE;step.fx2=fx;zmt_play_step(&step,voice);
            assert(zvb_peri_sound_select==voice);
        }
        step_t step;zmt_step_init(&step,0);zmt_process_fx(&step,FX_VOICE_TRI,voice);
        assert(zvb_peri_sound_select==voice && zvb_peri_sound_wave==(1|DUTY_CYCLE_50_0));
    }
    puts("volume effect checks passed");
}

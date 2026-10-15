#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

uint8_t zvb_peri_sound_freq_low, zvb_peri_sound_freq_high;
uint8_t zvb_peri_sound_wave, zvb_peri_sound_select, zvb_peri_sound_volume;
uint8_t zvb_peri_sound_master_vol, zvb_peri_sound_hold, zvb_config_dev_idx;
uint8_t zvb_peri_sound_left_channel, zvb_peri_sound_right_channel;
static uint8_t data[2048];
static uint16_t length, position;
static int opened;
static track_t track;
static pattern_t patterns[NUM_PATTERNS];
zos_dev_t open(const char* name, uint8_t flags) { position=0; opened++; return 1; }
zos_err_t close(zos_dev_t dev) { opened--; return ERR_SUCCESS; }
zos_err_t read(zos_dev_t dev, void* buf, uint16_t* size)
{
    if (*size > length-position) *size=length-position;
    memcpy(buf,data+position,*size);
    position+=*size;
    return ERR_SUCCESS;
}
void tracker_log_io_error(const char* operation,uint8_t step,zos_err_t err) {}

static void fixture(void)
{
    memset(data,0,sizeof data);
    memcpy(data,"ZMT",3);
    memcpy(data+4,"Test track  ",12);
    data[16]=32; data[17]=1;
    /* Arrangement zero uses pattern zero; voice zero has one C4 note. */
    data[18]=1; data[26]=0; data[27]=255;
    data[28]=1; data[29]=1; data[33]=1; data[34]=NOTE_MIDDLE_C;
    length=35;
    for(uint8_t p=0;p<NUM_PATTERNS;p++) track.patterns[p]=&patterns[p];
}
static void rejected(void)
{
    assert(zmt_file_load(&track,"test")!=ERR_SUCCESS);
    assert(opened==0);
}
int main(void)
{
    fixture();
    assert(zmt_file_load(&track,"test")==ERR_SUCCESS);
    assert(patterns[0].voices[0].steps[0].note==NOTE_MIDDLE_C);
    for(uint16_t size=0;size<35;size++) { fixture();length=size;rejected(); }
    fixture();data[0]='X';rejected();
    fixture();data[3]=1;rejected();
    fixture();data[16]=0;rejected();
    fixture();data[16]=3;rejected();
    fixture();data[16]=33;rejected();
    fixture();data[16]=132;rejected();
    fixture();data[17]=0;rejected();
    fixture();data[17]=17;rejected();
    fixture();data[26]=1;rejected();
    fixture();data[28]=0x80;rejected();
    fixture();data[33]=0x10;rejected();
    fixture();data[34]=200;rejected();
    fixture();data[33]=2;data[34]=4;rejected();
    fixture();data[34]=NOTE_OFF;assert(zmt_file_load(&track,"test")==ERR_SUCCESS);
    assert(opened==0);
    puts("tracker I/O tests passed");
}

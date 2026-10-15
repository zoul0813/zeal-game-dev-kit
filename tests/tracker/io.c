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
uint8_t tracker_mmu_page0;
static uint8_t backup[0x4000];
static int allocated, fail_alloc, fail_read, read_calls, fail_close;
void* tracker_test_memcpy(void* dst, const void* src, size_t size)
{
    void* result=dst;
    if ((uintptr_t)dst<sizeof backup) dst=backup+(uintptr_t)dst;
    if ((uintptr_t)src<sizeof backup) src=backup+(uintptr_t)src;
    memcpy(dst,src,size);
    return result;
}
zos_err_t palloc(uint8_t* page)
{
    if(fail_alloc) return ERR_NO_MORE_MEMORY;
    assert(!allocated); allocated=1;*page=7;return ERR_SUCCESS;
}
zos_err_t pfree(uint8_t page) { assert(page==7 && allocated);allocated=0;return ERR_SUCCESS; }
static int opened;
static int writes, fail_write, short_write;
static track_t track;
static pattern_t patterns[NUM_PATTERNS];
zos_dev_t open(const char* name, uint8_t flags) { position=0; read_calls=0; opened++; return 1; }
zos_err_t close(zos_dev_t dev) { opened--; return fail_close ? ERR_FAILURE : ERR_SUCCESS; }
zos_err_t read(zos_dev_t dev, void* buf, uint16_t* size)
{
    if (++read_calls==fail_read) return ERR_FAILURE;
    if (*size > length-position) *size=length-position;
    memcpy(buf,data+position,*size);
    position+=*size;
    return ERR_SUCCESS;
}
zos_err_t write(zos_dev_t dev, const void* buf, uint16_t* size)
{
    writes++;
    if (writes==fail_write) return ERR_FAILURE;
    if (writes==short_write) { *size=0; return ERR_SUCCESS; }
    return ERR_SUCCESS;
}
void log_error(const char* message, zos_err_t err) {}
void tracker_log_open_error(const char* message, const char* name, zos_err_t err) {}
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
    track_t previous=track;
    pattern_t previous_patterns[NUM_PATTERNS];
    memcpy(previous_patterns,patterns,sizeof patterns);
    zmt_current_pattern=5; zmt_ticks=11;zmt_next_step=9;
    tracker_mmu_page0=3;
    assert(zmt_file_load(&track,"test")!=ERR_SUCCESS);
    assert(memcmp(&previous,&track,sizeof track)==0);
    assert(memcmp(previous_patterns,patterns,sizeof patterns)==0);
    assert(zmt_current_pattern==5 && zmt_ticks==11 && zmt_next_step==9);
    assert(opened==0 && allocated==0 && tracker_mmu_page0==3);
}
static void test_failed_saves(void)
{
    fixture();
    assert(zmt_file_load(&track,"test")==ERR_SUCCESS);
    step_t* step=&patterns[0].voices[0].steps[0];
    step->waveform=WAV_SAWTOOTH;step->fx1=FX_COUNT_1;step->fx2=FX_GOTO_0;
    writes=0;
    assert(zmt_file_save(&track,"test")==ERR_SUCCESS);
    int total=writes;
    for(int i=1;i<=total;i++) {
        writes=0;fail_write=i;
        assert(zmt_file_save(&track,"test")!=ERR_SUCCESS);
        assert(opened==0);
        fail_write=0;writes=0;short_write=i;
        assert(zmt_file_save(&track,"test")!=ERR_SUCCESS);
        assert(opened==0);
        short_write=0;
    }
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
    fixture();fail_alloc=1;rejected();fail_alloc=0;
    fixture();fail_close=1;rejected();fail_close=0;
    for(int i=1;i<=11;i++) {fixture();fail_read=i;rejected();fail_read=0;}
    test_failed_saves();
    puts("tracker I/O tests passed");
}

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "zgdk/sound/tracker.h"
static uint8_t bytes[2048],expected[2048];
static unsigned calls,fail_at,zero_at,partial,opens,closes,pos;
static zos_err_t close_error;
zos_dev_t test_vfs_open(const char *p,uint8_t flags){assert(p && (flags&O_TRUNC));opens++;return 1;}
zos_err_t test_vfs_close(zos_dev_t dev){assert(dev==1);closes++;return close_error;}
zos_err_t test_vfs_write(zos_dev_t dev,const void *data,uint16_t *size){
    assert(dev==1);calls++;
    if(calls==fail_at)return ERR_FAILURE;
    if(calls==zero_at){*size=0;return ERR_SUCCESS;}
    if(partial && *size>1)*size=1;
    assert(pos+*size<=sizeof(bytes));memcpy(bytes+pos,data,*size);pos+=*size;return ERR_SUCCESS;
}
void log_error(const char *m,uint8_t e){(void)m;(void)e;}
void tracker_log_io_error(const char *m,uint8_t n,zos_err_t e){(void)m;(void)n;(void)e;}
void tracker_log_open_error(const char *m,const char *f,zos_err_t e){(void)m;(void)f;(void)e;}
static void reset(void){calls=fail_at=zero_at=partial=opens=closes=pos=0;close_error=ERR_SUCCESS;}
int main(void){
    static pattern_t pattern;track_t track={0};track.patterns[0]=&pattern;zmt_track_init(&track);
    step_t *step=&pattern.voices[0].steps[0];step->note=0;step->waveform=1;step->fx1=FX_COUNT_2;step->fx2=FX_GOTO_10;
    reset();assert(zmt_file_save(&track,"song.zmt")==ERR_SUCCESS);
    unsigned count=calls,length=pos;memcpy(expected,bytes,pos);assert(opens==1 && closes==1);
    for(unsigned i=1;i<=count;i++){
        reset();fail_at=i;assert(zmt_file_save(&track,"song.zmt")==ERR_FAILURE);
        assert(calls==i && closes==1);
        reset();zero_at=i;assert(zmt_file_save(&track,"song.zmt")==ERR_FAILURE);
        assert(calls==i && closes==1);
    }
    reset();partial=1;assert(zmt_file_save(&track,"song.zmt")==ERR_SUCCESS);
    assert(pos==length && memcmp(bytes,expected,length)==0 && closes==1);
    reset();close_error=ERR_INVALID_OFFSET;
    assert(zmt_file_save(&track,"song.zmt")==ERR_INVALID_OFFSET);
    reset();fail_at=2;close_error=ERR_INVALID_OFFSET;
    assert(zmt_file_save(&track,"song.zmt")==ERR_FAILURE && closes==1);
    reset();fail_at=2;assert(zmt_pattern_save(&pattern,1)==ERR_FAILURE);
    reset();track.pattern_count=17;assert(zmt_file_save(&track,"song.zmt")==ERR_INVALID_PARAMETER && opens==0);
    track.pattern_count=1;track.patterns[0]=NULL;
    assert(zmt_file_save(&track,"song.zmt")==ERR_INVALID_PARAMETER && opens==0);
    puts("tracker write checks passed");
}

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "zgdk/compress/rle.h"
int main(void) {
    uint8_t out[260], expected[260];
    uint8_t valid[] = {0x02,1,2,3,0x82,4,0x00,5};
    uint8_t wanted[] = {1,2,3,4,4,4,5};
    memset(out,0xa5,sizeof(out));
    assert(decompress_rle_checked(valid,sizeof(valid),out,7));
    assert(memcmp(out,wanted,7)==0 && out[7]==0xa5);
    memset(out,0xa5,sizeof(out)); memcpy(expected,out,sizeof(out));
    assert(!decompress_rle_checked(valid,sizeof(valid),out,6));
    assert(memcmp(out,expected,sizeof(out))==0);
    uint8_t truncated_literal[] = {0x7f,1};
    uint8_t truncated_run[] = {0x80};
    uint8_t trailing_header[] = {0x80,1,0x00};
    uint8_t *bad[] = {valid,truncated_literal,truncated_run,trailing_header};
    uint16_t sizes[] = {0,2,1,3};
    for (unsigned i=0;i<4;i++) {
        assert(!decompress_rle_checked(bad[i],sizes[i],out,sizeof(out)));
        decompress_rle(bad[i],sizes[i],out);
        assert(memcmp(out,expected,sizeof(out))==0);
    }
    assert(!decompress_rle_checked(NULL,1,out,sizeof(out)));
    assert(!decompress_rle_checked(valid,sizeof(valid),NULL,7));
    uint8_t max_literal[129];max_literal[0]=0x7f;
    for(unsigned i=1;i<129;i++)max_literal[i]=(uint8_t)i;
    assert(decompress_rle_checked(max_literal,129,out,128));
    assert(memcmp(out,max_literal+1,128)==0);
    uint8_t max_run[]={0xff,42,0xff,43};
    assert(decompress_rle_checked(max_run,4,out,256));
    assert(out[0]==42 && out[127]==42 && out[128]==43 && out[255]==43);
    uint8_t many[1024];for(unsigned i=0;i<1024;i+=2){many[i]=0xff;many[i+1]=1;}
    assert(!decompress_rle_checked(many,sizeof(many),out,UINT16_MAX));
    puts("RLE checks passed");
}

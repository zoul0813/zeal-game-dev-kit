#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "zgdk/tilemap.h"
static unsigned writes;
gfx_error gfx_tilemap_load(gfx_context*c,void*p,uint8_t n,uint8_t l,uint8_t x,uint8_t y){
    assert(c && p && n && x+n<=80 && y<40);writes++;return GFX_SUCCESS;
}
gfx_error gfx_tilemap_place(gfx_context*c,uint8_t t,uint8_t l,uint8_t x,uint8_t y){
    assert(c && x<80 && y<40);writes++;return GFX_SUCCESS;
}
int main(void){
    gfx_context ctx={0};Point p={0};uint8_t tiles[4]={10,20,30,40},expected[4];
    Tilemap map={.rect={.x=100,.y=200,.w=2,.h=2}};
    assert(tilemap_get(&p)==-1 && tilemap_set(&p,7)==-1);
    assert(tilemap_place(&ctx,0,1,&p)==GFX_INVALID_ARG);
    assert(tilemap_register(NULL,tiles)==ERR_INVALID_PARAMETER);
    assert(tilemap_register(&map,NULL)==ERR_INVALID_PARAMETER);
    assert(tilemap_register(&map,tiles)==ERR_SUCCESS);
    p=(Point){101,201};assert(tilemap_get(&p)==40 && tilemap_set(&p,41)==41);
    assert(tilemap_place(&ctx,0,1,&p)==GFX_SUCCESS);
    memcpy(expected,tiles,4);
    const Point bad[]={{99,200},{100,199},{102,200},{100,202},{356,200},{100,456},{65535,65535}};
    for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++){
        p=bad[i];assert(tilemap_get(&p)<0 && tilemap_set(&p,99)<0);
        assert(tilemap_place(&ctx,0,1,&p)==GFX_INVALID_ARG);
        assert(memcmp(tiles,expected,4)==0);
    }
    unsigned before=writes;
    assert(tilemap_place_xy(&ctx,0,1,80,0)==GFX_INVALID_ARG);
    assert(tilemap_place_xy(&ctx,0,1,0,40)==GFX_INVALID_ARG);
    assert(tilemap_fill(&ctx,0,1,79,39,2,1)==GFX_INVALID_ARG);
    assert(tilemap_fill(&ctx,0,1,0,39,1,2)==GFX_INVALID_ARG);
    assert(tilemap_fill(&ctx,0,1,256,0,1,1)==GFX_INVALID_ARG);
    assert(writes==before);
    assert(tilemap_fill(&ctx,0,1,79,39,1,1)==GFX_SUCCESS);
    assert(tilemap_fill(&ctx,0,1,0,0,80,40)==GFX_SUCCESS);
    puts("tilemap bounds checks passed");
}

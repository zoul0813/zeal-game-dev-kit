#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/sprites.h"
static gfx_sprite hardware[GFX_SPRITES_COUNT];
static int fail;
gfx_error gfx_sprite_render_array(gfx_context *ctx,uint8_t from,const gfx_sprite *s,uint8_t n){
    assert(ctx && s && n && (unsigned)from+n<=GFX_SPRITES_COUNT);
    if(fail)return GFX_INVALID_ARG;
    memcpy(hardware+from,s,n*sizeof(*s));return GFX_SUCCESS;
}
int main(void){
    gfx_context ctx={0};gfx_sprite arena[128],one[1],sprite={.x=32,.y=32,.tile=3,.flags=1};
    assert(sprites_register(sprite)==NULL);
    assert(sprites_register_arena(NULL,1)==GFX_INVALID_ARG);
    assert(sprites_register_arena(arena,0)==GFX_INVALID_ARG);
    assert(sprites_register_arena(arena,129)==GFX_INVALID_ARG);
    assert(sprites_register_arena(arena,128)==GFX_SUCCESS);
    for(unsigned i=0;i<128;i++)assert(sprites_register(sprite)==&arena[i]);
    assert(sprites_register(sprite)==NULL);
    assert(sprites_render(&ctx)==GFX_SUCCESS && hardware[127].tile==3);
    sprites_deregister();assert(sprites_register(sprite)==&arena[0]);
    fail=1;assert(sprites_render(&ctx)==GFX_INVALID_ARG);fail=0;
    assert(sprites_render(&ctx)==GFX_SUCCESS);
    assert(hardware[0].tile==3 && hardware[1].x==0 && hardware[127].flags==0);
    assert(sprites_register_arena(one,1)==GFX_SUCCESS);
    assert(sprites_register(sprite)==one && sprites_register(sprite)==NULL);
    sprites_deregister();assert(sprites_render(&ctx)==GFX_SUCCESS && hardware[0].x==0);
    assert(sprites_register(sprite)==one);
    assert(sprites_render(NULL)==GFX_INVALID_ARG);
    puts("sprite arena checks passed");
}

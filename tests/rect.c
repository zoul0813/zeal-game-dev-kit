#include <assert.h>
#include <stdio.h>
#include "zgdk/rect.h"
static uint32_t seed=123;
static uint16_t next(void){seed=seed*1664525u+1013904223u;return seed>>16;}
static int reference(Rect *a,Rect *b){
    return a->w && a->h && b->w && b->h &&
        (int32_t)a->x-(int32_t)a->w < b->x && (int32_t)b->x-(int32_t)b->w < a->x &&
        (int32_t)a->y-(int32_t)a->h < b->y && (int32_t)b->y-(int32_t)b->h < a->y;
}
int main(void){
    Rect a={.x=32,.y=32,.w=16,.h=16},b=a;
    assert(rect_collide(&a,&b));
    a=(Rect){.x=32,.y=32,.w=32,.h=32};b=(Rect){.x=16,.y=16,.w=8,.h=8};
    assert(rect_collide(&a,&b) && rect_collide(&b,&a));
    a=(Rect){.x=16,.y=16,.w=16,.h=16};b=(Rect){.x=32,.y=16,.w=16,.h=16};
    assert(!rect_collide(&a,&b));b.x--;assert(rect_collide(&a,&b));
    b.w=0;assert(!rect_collide(&a,&b));
    for(unsigned i=0;i<10000;i++){
        a=(Rect){.x=next(),.y=next(),.w=next(),.h=next()};
        b=(Rect){.x=next(),.y=next(),.w=next(),.h=next()};
        assert(rect_collide(&a,&b)==reference(&a,&b));
        assert(rect_collide(&a,&b)==rect_collide(&b,&a));
    }
    puts("rectangle overlap checks passed");
}

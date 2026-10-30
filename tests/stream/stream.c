#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/camera.h"
#include "zgdk/tilemap/stream.h"

static uint8_t vram[2][40][80];
static uint16_t scroll_x, scroll_y;
static unsigned writes, scrolls;
static uint32_t tiles_written;
static int gfx_fail = -1, read_fail = -1;
static int fail_now(int *counter) {
    if (*counter < 0) return 0;
    if (!*counter) { *counter = -1; return 1; }
    --*counter;
    return 0;
}
void tilemap_scroll(uint8_t layer, uint16_t x, uint16_t y) {
    assert(layer < 2 && x < 1280 && y < 640);
    scroll_x = x; scroll_y = y; scrolls++;
}
gfx_error gfx_tilemap_load(gfx_context *ctx, void *tiles, uint8_t n, uint8_t l, uint8_t x, uint8_t y) {
    (void)ctx;
    assert(l < 2 && x + n <= 80 && y < 40 && n);
    if (fail_now(&gfx_fail)) return GFX_INVALID_ARG;
    memcpy(&vram[l][y][x], tiles, n); writes++; tiles_written += n;
    return GFX_SUCCESS;
}
gfx_error gfx_tilemap_place(gfx_context *ctx, uint8_t tile, uint8_t l, uint8_t x, uint8_t y) {
    return gfx_tilemap_load(ctx, &tile, 1, l, x, y);
}
#define W 137
#define H 95
static uint8_t map[W*H];
static zos_err_t reader(void *src, uint16_t x, uint16_t y, uint8_t *dst, uint8_t n) {
    assert(src == map && x + n <= W && y < H && n);
    if (fail_now(&read_fail)) return ERR_INVALID_OFFSET;
    memcpy(dst, map + y * W + x, n);
    return ERR_SUCCESS;
}
static int floor_tile(int p) { return p >= 0 ? p / 16 : -1 - (-p - 1) / 16; }
static int ring(int t, int off, int n) { int r = (t + off) % n; return r < 0 ? r + n : r; }
/* Independent viewport model: check every tile actually exposed at this pixel
 * offset, rather than using the implementation's resident window dimensions. */
static void check(tilemap_stream_t *s, int px, int py) {
    int x, y;
    for (y = floor_tile(py); y <= floor_tile(py + s->view_h - 1); y++)
        for (x = floor_tile(px); x <= floor_tile(px + s->view_w - 1); x++) {
            if (x < 0 || y < 0 || x >= s->width || y >= s->height) continue;
            if (vram[s->layer][ring(y,s->ring_y,40)][ring(x,s->ring_x,80)] != s->tiles[(unsigned)y*s->width+x]) {
                fprintf(stderr,"view %u,%u pixel %d,%d loaded %d,%d ring %u,%u tile %d,%d got %u want %u\n",
                    s->view_w,s->view_h,px,py,s->tx,s->ty,s->ring_x,s->ring_y,x,y,
                    vram[s->layer][ring(y,s->ring_y,40)][ring(x,s->ring_x,80)],s->tiles[(unsigned)y*s->width+x]);
                assert(0);
            }
        }
}
/* Wide independent reference preserves the specified order: deadzone, movement
 * limit, final world clamp. Randomized comparisons protect narrow target math. */
static uint32_t random_state = 7;
static uint16_t random16(void) {
    random_state = random_state * 1664525U + 1013904223U;
    return (uint16_t)(random_state >> 16);
}
/* On the host, int is 32 bits, so the 16-bit code can't overflow here. Check
 * instead that every sum it computes would fit int16_t on the Z80 (SDCC int). */
static void fits16(int64_t v) { assert(v >= INT16_MIN && v <= INT16_MAX); }
static void follow_sums(const camera_t *c, int64_t tx, int64_t ty, int64_t w, int64_t h) {
    int64_t right=c->view_w-c->dz_right, bottom=c->view_h-c->dz_bottom;
    fits16(tx); fits16(ty); fits16(right); fits16(bottom);
    fits16(c->x+c->dz_left); fits16(tx-c->dz_left); fits16(tx+w); fits16(c->x+right); fits16(tx+w-right);
    fits16(c->y+c->dz_top); fits16(ty-c->dz_top); fits16(ty+h); fits16(c->y+bottom); fits16(ty+h-bottom);
    fits16(c->x+c->max_step); fits16(c->x-c->max_step); fits16(c->y+c->max_step); fits16(c->y-c->max_step);
}
static void follow_reference(camera_t *c, int16_t tx, int16_t ty,
                             uint16_t w, uint16_t h, int direction) {
    int64_t left=tx, top=ty, x=c->x, y=c->y;
    if(direction>0) left+=c->lead; else if(direction<0) left-=c->lead;
    follow_sums(c,left,top,w,h);
    if(left<x+c->dz_left) x=left-c->dz_left;
    else if(left+w>x+c->view_w-c->dz_right) x=left+w-c->view_w+c->dz_right;
    if(top<y+c->dz_top) y=top-c->dz_top;
    else if(top+h>y+c->view_h-c->dz_bottom) y=top+h-c->view_h+c->dz_bottom;
    if(c->max_step) {
        if(x>c->x+c->max_step) x=c->x+c->max_step;
        else if(x<c->x-c->max_step) x=c->x-c->max_step;
        if(y>c->y+c->max_step) y=c->y+c->max_step;
        else if(y<c->y-c->max_step) y=c->y-c->max_step;
    }
    if(x<0) x=0; else if(x>c->max_x) x=c->max_x;
    if(y<0) y=0; else if(y>c->max_y) y=c->max_y;
    c->x=(int16_t)x; c->y=(int16_t)y;
}
static void randomized_follow(void) {
    unsigned i;
    for(i=0;i<500000;i++) {
        camera_t c, ref;
        /* Every documented range at once: targets, sizes, views, margins, lead, step. */
        int16_t tx=(int16_t)(CAMERA_TARGET_MIN+random16()%(CAMERA_MAX_SIZE-CAMERA_TARGET_MIN));
        int16_t ty=(int16_t)(CAMERA_TARGET_MIN+random16()%(CAMERA_MAX_SIZE-CAMERA_TARGET_MIN));
        uint16_t w=random16()%(CAMERA_MAX_TARGET+1), h=random16()%(CAMERA_MAX_TARGET+1);
        int direction=(int)(random16()%3)-1;
        assert(camera_init_view(&c,1+random16()%CAMERA_MAX_SIZE,1+random16()%CAMERA_MAX_SIZE,
            1+random16()%CAMERA_MAX_SIZE,1+random16()%CAMERA_MAX_SIZE)==ERR_SUCCESS);
        c.x=c.max_x ? (int16_t)(random16()%(c.max_x+1)) : 0;
        c.y=c.max_y ? (int16_t)(random16()%(c.max_y+1)) : 0;
        c.lead=(uint8_t)random16(); c.max_step=(uint8_t)random16();
        c.dz_left=random16()%(c.view_w+1); c.dz_right=random16()%(c.view_w+1);
        c.dz_top=random16()%(c.view_h+1); c.dz_bottom=random16()%(c.view_h+1);
        ref=c;
        follow_reference(&ref,tx,ty,w,h,direction);
        camera_follow_target(&c,tx,ty,w,h,direction);
        assert(c.x==ref.x && c.y==ref.y);
    }
}
static void cameras(void) {
    camera_t c, before;
    unsigned i, shift;
    uint64_t total_x = 0, total_y = 0;
    assert(camera_init_view(&c, 2000, 1000, 320, 240) == ERR_SUCCESS);
    assert(c.dz_left == 106 && c.dz_top == 90);
    camera_set(&c, 400, 300);
    camera_follow_target(&c, 520, 400, 16, 16, 0);
    assert(c.x == 400 && c.y == 300);
    camera_follow_target(&c, 650, 500, 16, 16, 0);
    assert(c.x == 452 && c.y == 366);
    c.lead = 32; c.max_step = 5;
    camera_follow_target(&c, 1000, 900, 16, 16, 1);
    assert(c.x == 457 && c.y == 371);
    c.max_step = 0;
    camera_set(&c, 400, 300);
    camera_follow_target(&c, 600, 400, 16, 16, 1); assert(c.x == 434);
    camera_set(&c, 400, 300);
    camera_follow_target(&c, 510, 400, 16, 16, -1); assert(c.x == 372);
    /* The ends of the documented target ranges. */
    camera_follow_target(&c, CAMERA_TARGET_MIN, CAMERA_TARGET_MIN, CAMERA_MAX_TARGET, CAMERA_MAX_TARGET, -1);
    assert(c.x == 0 && c.y == 0);
    camera_follow_target(&c, CAMERA_MAX_SIZE - 1, CAMERA_MAX_SIZE - 1, CAMERA_MAX_TARGET, CAMERA_MAX_TARGET, 1);
    assert(c.x == c.max_x && c.y == c.max_y);
    before = c;
    assert(camera_init_view(&c,CAMERA_MAX_SIZE+1,1000,1,240)==ERR_INVALID_PARAMETER);
    assert(camera_init_view(&c,1000,1000,1,CAMERA_MAX_SIZE+1)==ERR_INVALID_PARAMETER);
    assert(camera_init_view(&c,65535,65535,65535,65535)==ERR_INVALID_PARAMETER);
    assert(memcmp(&c,&before,sizeof c)==0);
    assert(camera_init_view(&c,100,100,0,240)==ERR_INVALID_PARAMETER);
    assert(camera_init_view(&c,0,100,320,240)==ERR_INVALID_PARAMETER);
    assert(camera_init_view(&c,100,100,320,240)==ERR_SUCCESS);
    camera_follow_target(&c,300,300,32,32,0); assert(c.x==0 && c.y==0);
    assert(camera_init_view(&c,CAMERA_MAX_SIZE,CAMERA_MAX_SIZE,CAMERA_MAX_SIZE,CAMERA_MAX_SIZE)==ERR_SUCCESS);
    assert(c.max_x==0 && c.dz_left==5461 && c.dz_top==6143);
    assert(camera_init_view(&c,CAMERA_MAX_SIZE,CAMERA_MAX_SIZE,1,1)==ERR_SUCCESS);
    assert(c.max_x==CAMERA_MAX_SIZE-1 && c.dz_top==0);
    /* dz_top is floor(3 * vh / 8) for every view height */
    for(i=1;i<=CAMERA_MAX_SIZE;i++) {
        assert(camera_init_view(&c,100,100,320,i)==ERR_SUCCESS);
        assert(c.dz_top==i*3/8 && c.dz_bottom==i*3/8);
    }
    assert(camera_init_view(&c,CAMERA_MAX_SIZE,CAMERA_MAX_SIZE,1,1)==ERR_SUCCESS);
    camera_set(&c,CAMERA_MAX_SIZE,CAMERA_MAX_SIZE);
    assert(c.x==CAMERA_MAX_SIZE-1 && c.y==CAMERA_MAX_SIZE-1);
    c.max_step=255; c.dz_left=0; c.dz_top=0; c.lead=255;
    camera_follow_target(&c,CAMERA_MAX_SIZE-1,CAMERA_MAX_SIZE-1,CAMERA_MAX_TARGET,CAMERA_MAX_TARGET,1);
    assert(c.x==CAMERA_MAX_SIZE-1 && c.y==CAMERA_MAX_SIZE-1);
    c.lead=0;
    /* Cross the largest origin periods repeatedly. Every shift must agree with
     * an unbounded world-coordinate model, including fractional parallax bits. */
    assert(camera_init_view(&c,2000,1000,320,240)==ERR_SUCCESS);
    for(i=0;i<80000;i++) {
        camera_set(&c, 1000, 700);
        total_x += 1000; total_y += 700;
        assert(camera_rebase(&c,1000,700)==ERR_SUCCESS);
        assert(c.x==0 && c.y==0 && c.max_x==1680 && c.max_y==760);
        assert(c.origin_x < (1280U<<CAMERA_MAX_SHIFT) && c.origin_y < (640U<<CAMERA_MAX_SHIFT));
        if(i%137==0 || i==79999) for(shift=0;shift<=CAMERA_MAX_SHIFT;shift++) {
            assert(camera_scroll_layer(&c,1,shift,CAMERA_MAX_SHIFT-shift)==ERR_SUCCESS);
            assert(scroll_x==((total_x>>shift)%1280));
            assert(scroll_y==((total_y>>(CAMERA_MAX_SHIFT-shift))%640));
        }
    }
    /* Every local position and shift, from an origin near its limit. */
    for(i=0;i<=32767;i+=7) for(shift=0;shift<=CAMERA_MAX_SHIFT;shift++) {
        c.origin_x=(1280U<<CAMERA_MAX_SHIFT)-1; c.origin_y=(640U<<CAMERA_MAX_SHIFT)-1;
        c.x=(int16_t)i; c.y=(int16_t)i;
        assert(camera_scroll_layer(&c,0,shift,shift)==ERR_SUCCESS);
        assert(scroll_x==(((uint32_t)c.origin_x+i)>>shift)%1280);
        assert(scroll_y==(((uint32_t)c.origin_y+i)>>shift)%640);
    }
    /* The local position may not go negative. */
    camera_set(&c,0,0);
    before=c;
    assert(camera_rebase(&c,1,0)==ERR_INVALID_PARAMETER);
    assert(camera_rebase(&c,0,1)==ERR_INVALID_PARAMETER);
    assert(memcmp(&c,&before,sizeof c)==0);
    assert(camera_rebase(&c,0,0)==ERR_SUCCESS && !memcmp(&c,&before,sizeof c));
    c.x=INT16_MAX; c.y=INT16_MAX;
    before=c;
    assert(camera_rebase(&c,65535,0)==ERR_INVALID_PARAMETER);
    assert(memcmp(&c,&before,sizeof c)==0);
    assert(camera_rebase(&c,32767,32767)==ERR_SUCCESS);
    assert(c.x==0 && c.y==0);
    assert(c.origin_x < (1280U<<CAMERA_MAX_SHIFT) && c.origin_y < (640U<<CAMERA_MAX_SHIFT));
    c.x=-1; before=c;
    assert(camera_rebase(&c,0,0)==ERR_INVALID_PARAMETER && !memcmp(&c,&before,sizeof c));
    c.x=0;
    i=scrolls;
    assert(camera_scroll_layer(&c,2,0,0)==ERR_INVALID_PARAMETER);
    assert(camera_scroll_layer(&c,0,CAMERA_MAX_SHIFT+1,0)==ERR_INVALID_PARAMETER);
    assert(camera_scroll_layer(&c,0,0,CAMERA_MAX_SHIFT+1)==ERR_INVALID_PARAMETER && scrolls==i);
    for(i=1;i<=640;i*=2) {
        assert(camera_init_view(&c,2000,1000,i,i/2+1)==ERR_SUCCESS);
        camera_follow_target(&c,-100,320,400,500,0);
        assert(c.x>=0 && c.x<=c.max_x && c.y>=0 && c.y<=c.max_y);
    }
}
static void streaming(void) {
    tilemap_stream_t ram, remote, before;
    uint8_t scratch[80];
    int i, x, y, variant;
    unsigned n;
    static const uint16_t views[][2]={{320,240},{1,1},{16,16},{321,241},{1265,625},{73,39}};
    for(i=0;i<W*H;i++) map[i]=(uint8_t)(i*7+i/W);
    {
        uint32_t before_tiles;
        assert(tilemap_stream_init_ram(0,&ram,map,W,H,0,320,240,0,0)==ERR_SUCCESS);
        before_tiles=tiles_written;
        assert(tilemap_stream_update(0,&ram,15,15)==ERR_SUCCESS && tiles_written==before_tiles);
        assert(tilemap_stream_update(0,&ram,16,16)==ERR_SUCCESS);
        assert(tiles_written-before_tiles==21+16); /* one new row and column */
    }
    for(variant=0;variant<6;variant++) {
        memset(vram,0xCC,sizeof vram);
        assert(tilemap_stream_init_ram(0,&ram,map,W,H,0,views[variant][0],views[variant][1],0,0)==ERR_SUCCESS);
        assert(tilemap_stream_init_reader(0,&remote,reader,map,scratch,sizeof scratch,W,H,1,
            views[variant][0],views[variant][1],0,0)==ERR_SUCCESS);
        if(!variant) assert(ram.cols==21 && ram.rows==16);
        /* Deliberate edge/seam sweeps, negative positions, diagonal movement,
         * sub-tile offsets and teleports, in both directions. */
        for(i=0;i<3600;i++) {
            if(i<1200) { x=i*2-32; y=i-17; }
            else if(i<2400) { x=2400-(i-1200)*2; y=1200-(i-1200); }
            else { x=(i*157)%2300-48; y=(i*79)%1600-48; }
            assert(tilemap_stream_update(0,&ram,x,y)==ERR_SUCCESS);
            assert(tilemap_stream_update(0,&remote,x,y)==ERR_SUCCESS);
            check(&ram,x,y);
            assert(!memcmp(vram[0],vram[1],sizeof vram[0]));
        }
    }
    assert(tilemap_stream_init_ram(0,&ram,map,W,H,0,320,240,79*16,39*16)==ERR_SUCCESS);
    assert(tilemap_stream_init_reader(0,&remote,reader,map,scratch,21,W,H,1,320,240,79*16,39*16)==ERR_SUCCESS);
    assert(tilemap_stream_set_tile(0,&remote,0,0,3)==ERR_NOT_SUPPORTED);
    assert(tilemap_stream_shift_buffer(&remote,0,1)==ERR_NOT_SUPPORTED);
    assert(tilemap_stream_set_tile(0,&ram,80,40,0xED)==ERR_SUCCESS); check(&ram,79*16,39*16);
    assert(tilemap_stream_redraw(0,&remote)==ERR_SUCCESS);
    assert(!memcmp(vram[0],vram[1],sizeof vram[0]));
    assert(tilemap_stream_set_tile(0,&ram,W,0,0)==ERR_INVALID_PARAMETER);
    /* A failed incremental column after several writes must recover even if
     * the caller repeats the very same camera coordinates. */
    gfx_fail=3;
    assert(tilemap_stream_update(0,&ram,80*16,40*16)==ERR_FAILURE && ram.dirty);
    assert(tilemap_stream_update(0,&ram,80*16,40*16)==ERR_SUCCESS && !ram.dirty); check(&ram,80*16,40*16);
    read_fail=3;
    assert(tilemap_stream_update(0,&remote,80*16,40*16)==ERR_INVALID_OFFSET && remote.dirty);
    assert(tilemap_stream_update(0,&remote,80*16,40*16)==ERR_SUCCESS && !remote.dirty);
    assert(!memcmp(vram[0],vram[1],sizeof vram[0]));
    gfx_fail=2;
    assert(tilemap_stream_update(0,&ram,16,16)==ERR_FAILURE && ram.dirty);
    assert(tilemap_stream_update(0,&ram,32,48)==ERR_SUCCESS); check(&ram,32,48);
    gfx_fail=0;
    assert(tilemap_stream_set_tile(0,&ram,3,4,5)==ERR_FAILURE && ram.dirty);
    assert(tilemap_stream_update(0,&ram,32,48)==ERR_SUCCESS); check(&ram,32,48);
    gfx_fail=0;
    assert(tilemap_stream_init_ram(0,&ram,map,W,H,0,320,240,0,0)==ERR_FAILURE && ram.dirty);
    assert(tilemap_stream_update(0,&ram,0,0)==ERR_SUCCESS); check(&ram,0,0);
    read_fail=1;
    assert(tilemap_stream_init_reader(0,&remote,reader,map,scratch,21,W,H,1,320,240,0,0)==ERR_INVALID_OFFSET);
    assert(tilemap_stream_update(0,&remote,0,0)==ERR_SUCCESS && !remote.dirty);
    before=ram;
    n=writes;
    assert(tilemap_stream_init_ram(0,&ram,map,256,256,0,320,240,0,0)==ERR_INVALID_PARAMETER);
    assert(tilemap_stream_init_ram(0,&ram,map,W,H,2,320,240,0,0)==ERR_INVALID_PARAMETER);
    assert(tilemap_stream_init_ram(0,&ram,map,0,H,0,320,240,0,0)==ERR_INVALID_PARAMETER);
    assert(tilemap_stream_init_ram(0,&ram,map,W,H,0,1266,240,0,0)==ERR_INVALID_PARAMETER);
    assert(tilemap_stream_init_ram(0,&ram,map,W,H,0,320,626,0,0)==ERR_INVALID_PARAMETER);
    assert(tilemap_stream_init_reader(0,&ram,reader,map,scratch,20,W,H,0,320,240,0,0)==ERR_INVALID_PARAMETER);
    assert(!memcmp(&ram,&before,sizeof ram) && n==writes);
    assert(tilemap_stream_rebase(&ram,W,0)==ERR_INVALID_PARAMETER);
    assert(tilemap_stream_shift_buffer(&ram,0,H)==ERR_INVALID_PARAMETER);
    assert(!memcmp(&ram,&before,sizeof ram));
    ram.tx=INT16_MIN+1;
    assert(tilemap_stream_rebase(&ram,1,0)==ERR_SUCCESS && ram.tx==INT16_MIN);
    before=ram;
    assert(tilemap_stream_rebase(&ram,1,0)==ERR_INVALID_PARAMETER);
    assert(!memcmp(&ram,&before,sizeof ram));
    /* Outside-source cells are deliberately untouched. */
    memset(vram,0xCC,sizeof vram);
    assert(tilemap_stream_init_ram(0,&ram,map,2,2,0,320,240,-16,-16)==ERR_SUCCESS);
    assert(vram[0][39][79]==0xCC && vram[0][3][3]==0xCC);
    check(&ram,-16,-16);
    assert(tilemap_stream_update(0,&ram,INT16_MIN,INT16_MIN)==ERR_SUCCESS);
    assert(tilemap_stream_update(0,&ram,INT16_MAX,INT16_MAX)==ERR_SUCCESS);
}
static void compact_and_rebase(void) {
    uint8_t data[12*24], old[12*24];
    tilemap_stream_t s;
    camera_t c;
    unsigned base=0, i;
    int x,y;
    for(i=0;i<sizeof data;i++) data[i]=(uint8_t)i;
    memcpy(old,data,sizeof data);
    assert(tilemap_stream_init_ram(0,&s,data,12,24,0,100,100,0,0)==ERR_SUCCESS);
    assert(tilemap_stream_shift_buffer(&s,2,3)==ERR_SUCCESS);
    assert(s.tx==0 && s.ty==0 && s.ring_x==0 && s.ring_y==0);
    for(y=0;y<24;y++) for(x=0;x<12;x++)
        assert(data[y*12+x]==old[(y<21 && x<10)?(y+3)*12+x+2:y*12+x]);
    /* No entity, body, collision registry, or camera dependency in the stream.
     * This loop coordinates its independent rebase API with the camera API. */
    for(y=0;y<24;y++) for(x=0;x<12;x++) data[y*12+x]=(uint8_t)(y*3+x*5);
    assert(camera_init_view(&c,192,384,100,100)==ERR_SUCCESS);
    assert(tilemap_stream_init_ram(0,&s,data,12,24,0,100,100,0,0)==ERR_SUCCESS);
    for(i=0;i<20000;i++) {
        camera_set(&c,c.x+2,c.y+2);
        if(c.x>=64 && c.y>=64) {
            assert(tilemap_stream_shift_buffer(&s,1,1)==ERR_SUCCESS);
            assert(tilemap_stream_rebase(&s,1,1)==ERR_SUCCESS);
            assert(camera_rebase(&c,16,16)==ERR_SUCCESS);
            base++;
            for(y=0;y<24;y++) for(x=0;x<12;x++) if(x==11 || y==23)
                data[y*12+x]=(uint8_t)((base+y)*3+(base+x)*5);
        }
        assert(tilemap_stream_update(0,&s,c.x,c.y)==ERR_SUCCESS);
        check(&s,c.x,c.y);
        assert(camera_scroll_layer(&c,0,1,2)==ERR_SUCCESS);
        assert(scroll_x==(((base*16+c.x)>>1)%1280));
        assert(scroll_y==(((base*16+c.y)>>2)%640));
    }
    assert(base>1000);
}
int main(void) {
    cameras(); randomized_follow(); streaming(); compact_and_rebase();
    puts("camera/stream: all passed (sanitizers, viewport models, failure recovery, repeated rebases)");
    return 0;
}

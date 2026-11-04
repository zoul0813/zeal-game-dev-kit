#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "support.h"
#include <zgdk/rooms.h>
#include <zgdk/rooms/transitions.h>
#include <zgdk/rooms/slide.h>

static gfx_context graphics = { .bpp = 8 };
static uint8_t demo_tiles[300];
static uint8_t enigma_storage[842];
static unsigned loads, entries, frames, fail_load;
static uint16_t last_progress;
static uint8_t check_source, try_reentry;
static Vector2_u8 source;
static struct { uint8_t x,y,tx,ty; } log_entries[16];

static void reset_log(void) {
    loads=entries=frames=fail_load=last_progress=check_source=try_reentry=0;
    test_reset_graphics();
}

static zos_err_t load_room(gfx_context* ctx,uint8_t x,uint8_t y,uint8_t tx,uint8_t ty) {
    assert(ctx==&graphics && loads<16);
    log_entries[loads].x=x;log_entries[loads].y=y;log_entries[loads].tx=tx;log_entries[loads].ty=ty;
    loads++;
    if (check_source) assert(rooms_position.x==source.x && rooms_position.y==source.y);
    if (try_reentry) assert(rooms_move(EdgeRight)==-ERR_FAILURE);
    assert(tx+_TILEMAP->rect.w<=80 && ty+_TILEMAP->rect.h<=40);
    if (loads==fail_load) return ERR_ENTRY_CORRUPTED;
    for (uint8_t row=0;row<_TILEMAP->rect.h;row++) {
        for (uint8_t col=0;col<_TILEMAP->rect.w;col++) assert(tilemap_set_xy(col,row,x+8*y)>=0);
    }
    tilemap_set_xy(_TILEMAP->rect.w-1,_TILEMAP->rect.h-1,199);
    return ERR_SUCCESS;
}

static void enter_room(gfx_context* ctx,uint8_t x,uint8_t y,Edge edge) {
    assert(ctx==&graphics && rooms_position.x==x && rooms_position.y==y);
    assert(tilemap_get_xy(_TILEMAP->rect.w-1,_TILEMAP->rect.h-1)==199);
    assert(edge==EdgeNone || edge==EdgeLeft || edge==EdgeRight || edge==EdgeTop || edge==EdgeBottom);
    entries++;
    if (try_reentry) {
        assert(rooms_move(EdgeRight)==-ERR_FAILURE);
        assert(rooms_init(ctx,rooms_get_config(),rooms_position)==ERR_FAILURE);
    }
}

static void draw_frame(gfx_context* ctx,Edge edge,uint16_t progress) {
    assert(ctx==&graphics && progress==last_progress+4);
    assert(rooms_position.x==source.x && rooms_position.y==source.y);
    assert(edge==EdgeLeft || edge==EdgeRight || edge==EdgeTop || edge==EdgeBottom);
    last_progress=progress;frames++;
}
static const rooms_slide_config_t slide_config = { draw_frame };

static rooms_config_t demo_config(void) {
    rooms_config_t config = {
        .size={16,8},.screen={320,240},.room={0,0,20,15},.layer=0,
        .tiles=demo_tiles,.tiles_size=sizeof(demo_tiles),.load_room=load_room,.on_enter=enter_room,
        .transition=&ROOMS_TRANSITION_SLIDE,.transition_data=&slide_config,
    };
    return config;
}
static rooms_config_t enigma_config(void) {
    rooms_config_t config = {
        .size={8,8},.screen={640,480},.room={0,4,40,21},.layer=0,
        .tiles=enigma_storage+1,.tiles_size=840,.load_room=load_room,.on_enter=enter_room,
    };
    return config;
}

static void test_enigma(void) {
    rooms_config_t config=enigma_config();
    memset(enigma_storage,0xA5,sizeof(enigma_storage));
    reset_log();try_reentry=1;
    assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==ERR_SUCCESS);
    assert(loads==1 && entries==1 && frames==0);
    assert(log_entries[0].tx==0 && log_entries[0].ty==4);
    assert(test_scroll_x==0 && test_scroll_y==0);
    assert(_TILEMAP->rect.w==40 && _TILEMAP->rect.h==21 && _TILEMAP->rect.y==4);
    assert(tilemap_get_xy(39,20)==199 && tilemap_get_xy(40,0)<0 && tilemap_get_xy(0,21)<0);
    Point screen_point={39,24};assert(tilemap_get(&screen_point)==199);
    assert(enigma_storage[0]==0xA5 && enigma_storage[841]==0xA5);
    Edge edges[]={EdgeRight,EdgeLeft,EdgeTop,EdgeBottom};
    for (unsigned i=0;i<4;i++) {
        reset_log();source=rooms_position;check_source=1;
        assert(rooms_move(edges[i])==1);
        assert(loads==1 && entries==1 && frames==0);
        assert(log_entries[0].tx==0 && log_entries[0].ty==4);
    }
    reset_log();assert(rooms_init(&graphics,&config,(Vector2_u8){0,0})==0);
    reset_log();assert(rooms_move(EdgeLeft)==0 && rooms_move(EdgeTop)==0 && !loads && !entries);
    Vector2_u8 next;assert(!rooms_neighbor(EdgeLeft,&next) && !rooms_neighbor(EdgeRight,NULL));
    assert(rooms_move(EdgeContained)==-ERR_INVALID_PARAMETER);
    assert(rooms_init(&graphics,&config,(Vector2_u8){7,7})==0);
    reset_log();assert(rooms_move(EdgeRight)==0 && rooms_move(EdgeBottom)==0 && !loads && !entries);
}

static void test_slide(void) {
    rooms_config_t config=demo_config();
    reset_log();assert(rooms_init(&graphics,&config,(Vector2_u8){2,4})==0);
    assert(loads==1 && entries==1 && log_entries[0].tx==20 && log_entries[0].ty==0);
    assert(test_scroll_x==320 && test_scroll_y==0);
    Edge edges[]={EdgeRight,EdgeLeft,EdgeTop,EdgeBottom};
    uint8_t expected_x[]={40,0,20,20},expected_y[]={0,0,15,15};
    for (unsigned i=0;i<4;i++) {
        reset_log();source=rooms_position;check_source=1;
        assert(rooms_move(edges[i])==1);
        assert(entries==1 && loads==(edges[i]==EdgeTop ? 3u : 2u));
        assert(log_entries[0].tx==expected_x[i] && log_entries[0].ty==expected_y[i]);
        assert(log_entries[loads-1].tx==20 && log_entries[loads-1].ty==0);
        assert(frames==(i<2 ? 80u : 60u) && last_progress==(i<2 ? 320 : 240));
        assert(test_scroll_x==320 && test_scroll_y==0);
    }
}

static void test_failures(void) {
    rooms_config_t config=enigma_config();
    reset_log();assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==0);
    reset_log();source=rooms_position;fail_load=1;
    assert(rooms_move(EdgeRight)==-ERR_ENTRY_CORRUPTED);
    assert(entries==0 && rooms_position.x==source.x && rooms_position.y==source.y && rooms_get_config()==NULL);
    assert(rooms_move(EdgeRight)==-ERR_FAILURE && loads==1);
    reset_log();assert(rooms_init(&graphics,&config,source)==0 && entries==1);
    config=demo_config();
    for (unsigned failure=1;failure<=3;failure++) {
        reset_log();assert(rooms_init(&graphics,&config,(Vector2_u8){2,4})==0);
        reset_log();source=rooms_position;fail_load=failure;
        assert(rooms_move(EdgeTop)==-ERR_ENTRY_CORRUPTED);
        assert(entries==0 && rooms_position.x==source.x && rooms_position.y==source.y && !rooms_get_config());
    }
    reset_log();assert(rooms_init(&graphics,&config,(Vector2_u8){2,4})==0);
    reset_log();test_graphics_error=GFX_FAILURE;
    assert(rooms_move(EdgeRight)==-ERR_FAILURE && entries==0 && !rooms_get_config());
    test_reset_graphics();reset_log();fail_load=1;
    assert(rooms_init(&graphics,&config,(Vector2_u8){2,4})==ERR_ENTRY_CORRUPTED && !entries);
}

static void test_validation(void) {
    rooms_config_t config=enigma_config();
    reset_log();assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==0);
    reset_log();
    assert(rooms_init(NULL,&config,(Vector2_u8){2,3})==ERR_INVALID_PARAMETER);
    assert(rooms_init(&graphics,NULL,(Vector2_u8){2,3})==ERR_INVALID_PARAMETER);
    assert(rooms_init(&graphics,&config,(Vector2_u8){8,3})==ERR_INVALID_PARAMETER);
    config.tiles_size=839;assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==ERR_INVALID_PARAMETER);
    config=enigma_config();config.room.h=27;assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==ERR_INVALID_PARAMETER);
    config=enigma_config();config.screen.w=639;assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==ERR_INVALID_PARAMETER);
    config=enigma_config();config.size.w=0;assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==ERR_INVALID_PARAMETER);
    config=enigma_config();config.load_room=NULL;assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==ERR_INVALID_PARAMETER);
    config=enigma_config();config.tiles=NULL;assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==ERR_INVALID_PARAMETER);
    config=enigma_config();config.transition=&ROOMS_TRANSITION_SLIDE;
    assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==ERR_NOT_SUPPORTED);
    assert(!loads && !entries && rooms_get_config()!=NULL);
}

static unsigned custom_prepares,custom_runs;
static zos_err_t custom_prepare(const rooms_config_t* config,Vector2_u8* origin) {
    assert(config->room.h==21);origin->x=0;origin->y=0;custom_prepares++;return 0;
}
static zos_err_t custom_run(gfx_context* ctx,const rooms_config_t* config,const rooms_transition_request_t* request) {
    assert(ctx==&graphics && config->size.w==8 && request->from.x==2 && request->to.x==3 && request->edge==EdgeRight);
    custom_runs++;return 0;
}
static void test_custom_transition(void) {
    const rooms_transition_t custom={custom_prepare,custom_run};
    rooms_config_t config=enigma_config();config.transition=&custom;
    reset_log();assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==0 && custom_prepares==1);
    reset_log();assert(rooms_move(EdgeRight)==1 && custom_runs==1 && entries==1 && loads==1);
    config=enigma_config();config.transition=NULL;
    assert(rooms_init(&graphics,&config,(Vector2_u8){2,3})==0);
}

int main(void) {
    test_validation();test_enigma();test_slide();test_failures();test_custom_transition();
    puts("PASS: world geometry, Enigma viewport, slide staging, entry callbacks, boundaries, reentrancy and failures");
    return 0;
}

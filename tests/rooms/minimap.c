#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zgdk/minimap.h>
#include "support.h"

static uint8_t visited[20],special[20];
static minimap_state_config_t state={ .size={8,8},.visited=visited,.special=special,.capacity=8 };
static minimap_render_config_t renderer={ .position={38,0},.layer=1,.first_tile=200,.colors={2,3,4,5} };
static uint8_t pixel(uint8_t x,uint8_t y,uint8_t bpp) {
    uint16_t offset=(uint16_t)(x/16)*256+(uint16_t)y*16+x%16;
    if (bpp==8) return test_uploaded[offset];
    return offset&1 ? test_uploaded[offset/2]&15 : test_uploaded[offset/2]>>4;
}
static void test_storage_overlap(void) {
    uint8_t storage[24], before[24];
    memset(storage,0xA5,sizeof(storage));
    minimap_state_config_t config = {.size={8,8}, .visited=storage, .special=storage+8, .capacity=16};
    /* Adjacent active spans are valid even when unused capacity overlaps. */
    assert(minimap_state_init(&config,(Vector2_u8){7,7})==ERR_SUCCESS);
    assert(!minimap_room_is_special(7,6));
    memcpy(before,storage,sizeof(storage));
    config.special=storage+1;
    assert(minimap_state_init(&config,(Vector2_u8){0,0})==ERR_INVALID_PARAMETER);
    assert(memcmp(before,storage,sizeof(storage))==0);
    config.visited=storage+1;config.special=storage;
    assert(minimap_state_init(&config,(Vector2_u8){0,0})==ERR_INVALID_PARAMETER);
    assert(memcmp(before,storage,sizeof(storage))==0);
    config.visited=storage;config.special=storage;
    assert(minimap_state_init(&config,(Vector2_u8){0,0})==ERR_INVALID_PARAMETER);
    assert(minimap_get_config()->special==storage+8 && minimap_room_visited(7,7));
    /* Leave no references to stack-owned test buffers behind. */
    assert(minimap_state_init(&state,(Vector2_u8){0,0})==ERR_SUCCESS);
}

static void test_invalid_init_preserves_state(void) {
    gfx_context graphics={.bpp=4};
    minimap_state_config_t good_state={{8,8},visited,special,8};
    minimap_render_config_t good_render={{0,0},1,20,{2,3,4,5}};
    uint8_t saved_visited[20],saved_special[20];
    assert(minimap_init(&graphics,&good_state,&good_render,(Vector2_u8){0,0})==GFX_SUCCESS);
    assert(minimap_set_position(&graphics,(Vector2_u8){1,0},1)==GFX_SUCCESS);
    memcpy(saved_visited,visited,sizeof(visited));memcpy(saved_special,special,sizeof(special));
    test_reset_graphics();
    for (unsigned failure=0;failure<10;failure++) {
        minimap_render_config_t bad=good_render;
        minimap_state_config_t candidate=good_state;
        Vector2_u8 start={0,0};
        switch (failure) {
            case 0: bad.first_tile=255;break;
            case 1: bad.layer=2;break;
            case 2: bad.position.x=79;break;
            case 3: bad.position.y=40;break;
            case 4: bad.colors.current=16;break;
            case 5: candidate.size.w=17;candidate.capacity=20;break;
            case 6: candidate.size.h=9;candidate.capacity=20;break;
            case 7: candidate.special=visited;break;
            case 8: start.x=8;break;
            case 9: graphics.bpp=1;break;
        }
        assert(minimap_init(&graphics,&candidate,&bad,start)==GFX_INVALID_ARG);
        graphics.bpp=4;
        assert(!memcmp(saved_visited,visited,sizeof(visited)));
        assert(!memcmp(saved_special,special,sizeof(special)));
        Vector2_u8 current=minimap_get_position();assert(current.x==1 && current.y==0);
        assert(minimap_get_config()->size.w==8 && minimap_get_config()->size.h==8);
        assert(!test_upload_count && !test_place_count);
    }
    /* A rejected init must also retain the working convenience renderer. */
    assert(minimap_set_position(&graphics,(Vector2_u8){2,0},0)==GFX_SUCCESS);
}

static void test_failed_update_retry(void) {
    gfx_context graphics={.bpp=8};
    minimap_state_config_t config={{8,8},visited,special,8};
    minimap_render_config_t render={{0,0},1,20,{2,3,4,5}};
    uint8_t saved_visited[20],saved_special[20];
    for (unsigned separate_byte=0;separate_byte<2;separate_byte++) {
        Vector2_u8 destination={1,separate_byte};
        unsigned index=destination.y*8+destination.x;
        for (unsigned previous_special=0;previous_special<2;previous_special++) {
            for (unsigned revisiting=0;revisiting<2;revisiting++) {
                assert(minimap_init(&graphics,&config,&render,(Vector2_u8){0,0})==GFX_SUCCESS);
                if (!previous_special) special[0]|=1; /* Exercise rollback of clearing a flag too. */
                if (revisiting) {
                    visited[index>>3]|=1<<(index&7);
                    special[index>>3]|=1<<(index&7);
                }
                memcpy(saved_visited,visited,sizeof(visited));memcpy(saved_special,special,sizeof(special));
                test_graphics_error=GFX_FAILURE;
                assert(minimap_set_position(&graphics,destination,previous_special)==GFX_FAILURE);
                assert(!memcmp(saved_visited,visited,sizeof(visited)));
                assert(!memcmp(saved_special,special,sizeof(special)));
                Vector2_u8 current=minimap_get_position();assert(current.x==0 && current.y==0);
                test_graphics_error=GFX_SUCCESS;
                assert(minimap_set_position(&graphics,destination,previous_special)==GFX_SUCCESS);
                assert(!!minimap_room_is_special(0,0)==previous_special);
                assert(!!minimap_room_is_special(destination.x,destination.y)==revisiting);
                assert(minimap_room_visited(destination.x,destination.y));
                current=minimap_get_position();assert(current.x==destination.x && current.y==destination.y);
                assert(pixel(0,0,8)==(previous_special ? 4 : 3));
                assert(pixel(2,destination.y*2,8)==5);
            }
        }
    }
}

int main(void) {
    gfx_context graphics={.bpp=4};
    assert(minimap_render(&graphics,&renderer)==GFX_INVALID_ARG);
    memset(visited,0xA5,sizeof(visited));memset(special,0xA5,sizeof(special));
    assert(minimap_init(&graphics,&state,&renderer,(Vector2_u8){2,3})==0);
    assert(test_uploaded_size==256 && test_uploaded_from==200*128);
    assert(test_placed_x[0]==38 && test_placed_x[1]==39 && test_placed_tile[1]==201 && test_placed_layer[0]==1);
    assert(visited[8]==0xA5 && special[8]==0xA5);
    assert(pixel(4,6,4)==5 && pixel(5,7,4)==5 && pixel(6,6,4)==2);
    assert(minimap_set_position(&graphics,(Vector2_u8){3,3},1)==0);
    assert(pixel(4,6,4)==4 && pixel(6,6,4)==5);
    assert(minimap_set_position(&graphics,(Vector2_u8){2,3},0)==0);
    assert(minimap_set_position(&graphics,(Vector2_u8){3,3},0)==0);
    assert(pixel(4,6,4)==3 && pixel(6,6,4)==5 && pixel(31,15,4)==2);
    assert(!minimap_room_is_special(2,3) && minimap_room_visited(2,3));
    assert(minimap_state_set_position((Vector2_u8){8,0},1)==ERR_INVALID_PARAMETER);
    Vector2_u8 before=minimap_get_position();assert(before.x==3 && before.y==3);
    assert(!minimap_room_visited(255,255));
    renderer.colors.current=16;assert(minimap_render(&graphics,&renderer)==GFX_INVALID_ARG);renderer.colors.current=5;
    state.size=(Size_u8){16,8};state.capacity=16;graphics.bpp=8;renderer.first_tile=253;renderer.position=(Vector2_u8){18,0};
    renderer.colors=(minimap_colors_t){23,27,36,16};
    assert(minimap_init(&graphics,&state,&renderer,(Vector2_u8){9,7})==0);
    assert(test_uploaded_size==512 && test_uploaded_from==253*256 && pixel(18,14,8)==16 && pixel(19,15,8)==16);
    state.size=(Size_u8){17,9};state.capacity=20;
    assert(minimap_state_init(&state,(Vector2_u8){16,8})==0 && minimap_room_visited(16,8));
    assert(minimap_render(&graphics,&renderer)==GFX_INVALID_ARG);
    state.capacity=19;assert(minimap_state_init(&state,(Vector2_u8){16,8})==ERR_INVALID_PARAMETER);
    assert(minimap_state_init(NULL,(Vector2_u8){0,0})==ERR_INVALID_PARAMETER);
    state.size=(Size_u8){8,8};state.capacity=8;
    test_graphics_error=GFX_FAILURE;
    assert(minimap_init(&graphics,&state,&renderer,(Vector2_u8){0,0})==GFX_FAILURE);
    assert(minimap_set_position(&graphics,(Vector2_u8){1,0},0)==GFX_INVALID_ARG);
    test_graphics_error=0;
    assert(minimap_init(&graphics,&state,&renderer,(Vector2_u8){0,0})==0);
    assert(minimap_init(NULL,&state,&renderer,(Vector2_u8){0,0})==GFX_INVALID_ARG);
    test_storage_overlap();
    test_invalid_init_preserves_state();
    test_failed_update_retry();
    puts("PASS: runtime minimap dimensions/storage, generic markers, game colors and placement, 4/8-bit packing, validation and graphics errors");
    return 0;
}

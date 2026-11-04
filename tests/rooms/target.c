/* Compile/link probe only: graphics hardware is not initialized here. */
#include <stddef.h>
#include <zgdk/rooms.h>
#include <zgdk/rooms/transitions.h>
#ifdef PROBE_SLIDE
#include <zgdk/rooms/slide.h>
#endif
#ifdef PROBE_MINIMAP
#include <zgdk/minimap/state.h>
static uint8_t visited[8], special[8];
static const minimap_state_config_t state = { {8,8}, visited, special, 8 };
int main(void) {
    Vector2_u8 start = {2,3};
    return minimap_state_init(&state, start);
}
#else
#ifdef PROBE_SLIDE
#define ROOM_COLUMNS 20
#define ROOM_ROWS 15
#else
#define ROOM_COLUMNS 40
#define ROOM_ROWS 21
#endif
static uint8_t tiles[ROOM_COLUMNS * ROOM_ROWS];
static zos_err_t load_room(gfx_context* ctx,uint8_t x,uint8_t y,uint8_t tile_x,uint8_t tile_y) {
    (void)ctx;
    if ((uint16_t)tile_x + ROOM_COLUMNS > 80 || (uint16_t)tile_y + ROOM_ROWS > 40) return ERR_INVALID_PARAMETER;
    for (uint8_t row=0;row<ROOM_ROWS;row++) {
        for (uint8_t col=0;col<ROOM_COLUMNS;col++) tilemap_set_xy(col,row,x+y*8);
    }
    return ERR_SUCCESS;
}
/* Export both the emitted initializer and the compiler's expected field
 * offsets so check-target.py can catch target-only initializer corruption. */
const uint8_t probe_offsets[] = {
    sizeof(rooms_config_t), offsetof(rooms_config_t, size),
    offsetof(rooms_config_t, screen), offsetof(rooms_config_t, room),
    offsetof(rooms_config_t, layer), offsetof(rooms_config_t, tiles),
    offsetof(rooms_config_t, tiles_size), offsetof(rooms_config_t, load_room),
    offsetof(rooms_config_t, transition),
};
const rooms_config_t probe_config = {
    .size = {8,8},
#ifdef PROBE_SLIDE
    .screen = {320,240}, .room = {.x=0,.y=0,.w=ROOM_COLUMNS,.h=ROOM_ROWS},
    .transition = &ROOMS_TRANSITION_SLIDE,
#else
    .screen = {640,480}, .room = {.x=0,.y=4,.w=ROOM_COLUMNS,.h=ROOM_ROWS},
#endif
    .tiles = tiles, .tiles_size = sizeof(tiles), .load_room = load_room,
};
int main(void) {
    gfx_context ctx;
    Vector2_u8 start = {2,3};
    zos_err_t err = rooms_init(&ctx, &probe_config, start);
    return err == ERR_SUCCESS ? rooms_move(EdgeRight) : err;
}
#endif

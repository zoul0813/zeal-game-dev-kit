#include <zgdk/rooms.h>
#include "internal.h"

#include <zgdk/tilemap/scroll.h>

Vector2_u8 rooms_position;
rooms_runtime_t rooms_runtime;

static uint8_t valid_config(const rooms_config_t* config, Vector2_u8 start) {
    if (config == NULL || config->tiles == NULL || config->load_room == NULL) return 0;
    if (!config->size.w || !config->size.h || start.x >= config->size.w || start.y >= config->size.h) return 0;
    if (!config->screen.w || !config->screen.h || config->screen.w % ROOMS_TILE_PIXELS || config->screen.h % ROOMS_TILE_PIXELS) return 0;
    uint16_t columns = config->screen.w / ROOMS_TILE_PIXELS;
    uint16_t rows = config->screen.h / ROOMS_TILE_PIXELS;
    if (columns > ROOMS_MAP_COLUMNS || rows > ROOMS_MAP_ROWS || config->layer > 1) return 0;
    if (!config->room.w || !config->room.h || config->room.x >= columns || config->room.y >= rows) return 0;
    if (config->room.w > columns - config->room.x || config->room.h > rows - config->room.y) return 0;
    if (config->tiles_size < config->room.w * config->room.h) return 0;
    if (config->transition != NULL && config->transition->run == NULL) return 0;
    return 1;
}

static zos_err_t load_visible(Vector2_u8 destination) {
    const rooms_config_t* config = &rooms_runtime.config;
    return config->load_room(rooms_runtime.ctx, destination.x, destination.y,
        rooms_runtime.origin.x + config->room.x, rooms_runtime.origin.y + config->room.y);
}

static zos_err_t present(void) {
    if (gfx_wait_vblank(rooms_runtime.ctx) != GFX_SUCCESS) return ERR_FAILURE;
    tilemap_scroll(rooms_runtime.config.layer,
        (uint16_t)rooms_runtime.origin.x * ROOMS_TILE_PIXELS,
        (uint16_t)rooms_runtime.origin.y * ROOMS_TILE_PIXELS);
    if (gfx_wait_end_vblank(rooms_runtime.ctx) != GFX_SUCCESS) return ERR_FAILURE;
    return ERR_SUCCESS;
}

static zos_err_t finish_entry(Vector2_u8 destination, Edge edge, zos_err_t err) {
    if (err == ERR_SUCCESS) err = present();
    if (err == ERR_SUCCESS) {
        rooms_position = destination;
        rooms_runtime.ready = 1;
        if (rooms_runtime.config.on_enter != NULL) {
            rooms_runtime.config.on_enter(rooms_runtime.ctx, destination.x, destination.y, edge);
        }
    } else {
        rooms_runtime.ready = 0;
    }
    rooms_runtime.busy = 0;
    return err;
}

zos_err_t rooms_init(gfx_context* ctx, const rooms_config_t* config, Vector2_u8 start) {
    if (rooms_runtime.busy) return ERR_FAILURE;
    if (ctx == NULL || !valid_config(config, start)) return ERR_INVALID_PARAMETER;

    Vector2_u8 origin = {0, 0};
    zos_err_t err = ERR_SUCCESS;
    rooms_runtime.busy = 1;
    if (config->transition != NULL && config->transition->prepare != NULL) {
        err = config->transition->prepare(config, &origin);
    }
    if (err == ERR_SUCCESS &&
        ((uint16_t)origin.x + config->screen.w / ROOMS_TILE_PIXELS > ROOMS_MAP_COLUMNS ||
         (uint16_t)origin.y + config->screen.h / ROOMS_TILE_PIXELS > ROOMS_MAP_ROWS)) {
        err = ERR_INVALID_PARAMETER;
    }
    if (err != ERR_SUCCESS) {
        rooms_runtime.busy = 0;
        return err;
    }

    rooms_runtime.ready = 0;
    rooms_runtime.ctx = ctx;
    rooms_runtime.config = *config;
    rooms_runtime.origin = origin;
    rect_set_pointer(&rooms_runtime.map.rect, config->room.x, config->room.y, config->room.w, config->room.h);
    err = tilemap_register(&rooms_runtime.map, config->tiles);
    if (err == ERR_SUCCESS) err = load_visible(start);
    return finish_entry(start, EdgeNone, err);
}

zos_err_t rooms_change_room(Vector2_u8 destination, Edge edge) {
    if (!rooms_runtime.ready || rooms_runtime.busy) return ERR_FAILURE;
    rooms_transition_request_t request;
    request.from = rooms_position;
    request.to = destination;
    request.origin = rooms_runtime.origin;
    request.edge = edge;
    rooms_runtime.busy = 1;
    zos_err_t err = ERR_SUCCESS;
    if (rooms_runtime.config.transition != NULL) {
        err = rooms_runtime.config.transition->run(rooms_runtime.ctx, &rooms_runtime.config, &request);
    }
    if (err == ERR_SUCCESS) err = load_visible(destination);
    return finish_entry(destination, edge, err);
}

const rooms_config_t* rooms_get_config(void) {
    return rooms_runtime.ready ? &rooms_runtime.config : NULL;
}

void rooms_get_origin(Vector2_u8* origin) {
    if (origin != NULL) *origin = rooms_runtime.origin;
}

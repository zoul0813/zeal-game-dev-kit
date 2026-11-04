#ifndef ROOMS_INTERNAL_H
#define ROOMS_INTERNAL_H

#include <zgdk/rooms.h>

typedef struct {
    gfx_context* ctx;
    rooms_config_t config;
    Tilemap map;
    Vector2_u8 origin;
    uint8_t ready;
    uint8_t busy;
} rooms_runtime_t;

extern rooms_runtime_t rooms_runtime;
zos_err_t rooms_change_room(Vector2_u8 destination, Edge edge);

#endif

#ifndef ZGDK_ROOMS_TRANSITIONS_H
#define ZGDK_ROOMS_TRANSITIONS_H

#include <zgdk/rooms.h>

/* No entity positioning or drawing. Boundary/invalid queries return 0. */
uint8_t rooms_neighbor(Edge edge, Vector2_u8* destination);
/* 1 = entered neighbor, 0 = world boundary, negative = negated load/error code. */
int16_t rooms_move(Edge edge);

#endif

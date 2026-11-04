#ifndef ZGDK_ROOMS_SLIDE_H
#define ZGDK_ROOMS_SLIDE_H

#include <zgdk/rooms.h>

typedef struct {
    /* Optional game drawing during vblank, after each scroll step.
     * progress is the number of pixels scrolled in the exit direction. */
    void (*draw_frame)(gfx_context* ctx, Edge edge, uint16_t progress);
} rooms_slide_config_t;

/* Full-screen sliding, using three horizontal and two vertical staging screens.
 * prepare rejects layouts that cannot fit in ZVB's 80x40 tilemap.
 * Pass optional rooms_slide_config_t through config.transition_data. */
extern const rooms_transition_t ROOMS_TRANSITION_SLIDE;

#endif

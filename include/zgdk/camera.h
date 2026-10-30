#ifndef ZGDK_CAMERA_H
#define ZGDK_CAMERA_H
#include <stdint.h>
#include <zos_errors.h>

/* Local top-left pixel coordinates, independent of physics and Rect conventions.
 * Fields may be tuned after initialization. Keep deadzone margins within the view.
 * origin_* retain the ordinary hardware phase; phase_* retain parallax phase. */
typedef struct {
    int16_t x, y;
    int16_t max_x, max_y;
    uint16_t view_w, view_h;
    uint16_t dz_left, dz_right, dz_top, dz_bottom;
    uint8_t lead, max_step;
    uint16_t origin_x, origin_y;
    uint32_t phase_x, phase_y;
} camera_t;

/* Nonzero dimensions; world minus view must fit in INT16_MAX on each axis.
 * Invalid initialization leaves the destination unchanged. */
zos_err_t camera_init_view(camera_t *camera, uint16_t world_w, uint16_t world_h,
                           uint16_t view_w, uint16_t view_h);
void camera_set(camera_t *camera, int16_t x, int16_t y);
/* Target coordinates are the top-left, direction is negative/zero/positive. */
void camera_follow_target(camera_t *camera, int16_t x, int16_t y,
                          uint16_t width, uint16_t height, int8_t direction);
/* layer 0 or 1; shifts 0..15. Invalid arguments perform no hardware writes. */
zos_err_t camera_scroll_layer(const camera_t *camera, uint8_t layer,
                              uint8_t shift_x, uint8_t shift_y);
/* Subtract nonnegative offsets without changing bounds. The new local position
 * must fit int16_t. Atomic on invalid input. Zero offsets are allowed. */
zos_err_t camera_rebase(camera_t *camera, uint16_t dx, uint16_t dy);
#endif

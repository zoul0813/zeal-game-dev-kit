#ifndef ZGDK_CAMERA_H
#define ZGDK_CAMERA_H
#include <stdint.h>
#include <zos_errors.h>

/* Limits. All camera math is 16-bit; these ranges keep following intermediates
 * inside int16_t and scroll/rebase intermediates inside uint16_t. Validation
 * is listed below; unchecked inputs outside their ranges may give a wrong position.
 *
 *   world and view size     1..CAMERA_MAX_SIZE px per axis (camera_init_view checks)
 *   camera x, y             0..CAMERA_MAX_SIZE - 1 (camera_set clamps; do not
 *                           write larger values into the fields yourself)
 *   follow target x, y      CAMERA_TARGET_MIN..CAMERA_MAX_SIZE - 1 px (top-left),
 *                           in the same local coordinates as the camera
 *   follow target w, h      0..CAMERA_MAX_TARGET px
 *   deadzone margins        0..view size on that axis; lead and max_step 0..255
 *   parallax shift          0..CAMERA_MAX_SHIFT (camera_scroll_layer checks)
 *
 * 16383 px is 1023 whole tiles of 16 px per axis; 1023 x 64 tiles uses 65472 bytes, just under 64 KiB.
 * Longer worlds stream and rebase (camera_rebase), so the camera only ever sees
 * the local window. Signed 12.4 positions have at most 2047 whole pixels. */
#define CAMERA_MAX_SIZE   16383
#define CAMERA_TARGET_MIN (-8192)
#define CAMERA_MAX_TARGET 8191
/* Largest parallax shift for camera_scroll_layer (1/16 speed). */
#define CAMERA_MAX_SHIFT 4

/* Local top-left pixel coordinates, independent of physics and Rect conventions.
 * Tune deadzone margins, lead, and max_step after initialization; other fields
 * are library-managed. Keep deadzone margins within the view.
 * See the limits above for x, y. origin_* hold the pixels removed by camera_rebase, mod
 * 1280 << CAMERA_MAX_SHIFT and 640 << CAMERA_MAX_SHIFT, which keeps every
 * supported parallax shift exact after any number of rebases. */
typedef struct {
    int16_t x, y;
    int16_t max_x, max_y;
    uint16_t view_w, view_h;
    uint16_t dz_left, dz_right, dz_top, dz_bottom;
    uint8_t lead, max_step;
    uint16_t origin_x, origin_y;
} camera_t;

/* World and view sizes 1..CAMERA_MAX_SIZE. Invalid initialization leaves the
 * destination unchanged. */
zos_err_t camera_init_view(camera_t *camera, uint16_t world_w, uint16_t world_h,
                           uint16_t view_w, uint16_t view_h);
void camera_set(camera_t *camera, int16_t x, int16_t y);
/* Target coordinates are the top-left, direction is negative/zero/positive.
 * Position and size must be in the ranges above. */
void camera_follow_target(camera_t *camera, int16_t x, int16_t y,
                          uint16_t width, uint16_t height, int8_t direction);
/* layer 0 or 1; shifts 0..CAMERA_MAX_SHIFT. Invalid arguments perform no hardware writes. */
zos_err_t camera_scroll_layer(const camera_t *camera, uint8_t layer,
                              uint8_t shift_x, uint8_t shift_y);
/* Subtract offsets without changing bounds. Offsets may not exceed the current
 * position, so it stays nonnegative. Atomic on invalid input. Zero offsets are allowed. */
zos_err_t camera_rebase(camera_t *camera, uint16_t dx, uint16_t dy);
#endif

#include "zgdk/camera.h"
#include "zgdk/tilemap/scroll.h"

static uint16_t scroll_axis(int16_t local, uint16_t origin, uint16_t phase,
                            uint8_t shift, uint16_t period) {
    int32_t value;
    if (!shift) {
        int16_t ordinary = local % (int16_t)period;
        ordinary += origin;
        if (ordinary < 0) ordinary += period;
        if (ordinary >= period) ordinary -= period;
        return (uint16_t)ordinary;
    }
    value = (int32_t)((uint32_t)phase * period) + origin + local;
    if (value < 0) value += (uint32_t)period << 15;
    {
        uint32_t divided = (uint32_t)value >> shift;
        uint16_t result = (uint16_t)divided % period;
        /* 65536 == 256 (mod 1280) and (mod 640), and five high
         * words are a multiple of both periods. Fold to 16 bits so Z80
         * scrolling needs no 32-bit division runtime. */
        result += ((uint16_t)(divided >> 16) % 5) << 8;
        while (result >= period) result -= period;
        return result;
    }
}
zos_err_t camera_scroll_layer(const camera_t *c, uint8_t layer, uint8_t sx, uint8_t sy) {
    if (!c || layer > 1 || sx > 15 || sy > 15) return ERR_INVALID_PARAMETER;
    tilemap_scroll(layer, scroll_axis(c->x, c->origin_x, (uint16_t)c->phase_x, sx, 1280),
                          scroll_axis(c->y, c->origin_y, (uint16_t)c->phase_y, sy, 640));
    return ERR_SUCCESS;
}

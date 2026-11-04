#include <zgdk/entities/sprite.h>
/* Internal ASM inputs: origin correction and camera in 12.4 units. */
uint16_t _entity_view_limit_x = 320 + 15;
uint16_t _entity_view_limit_y = 240 + 15;
uint16_t _entity_view_off_x = 16 * 16;
uint16_t _entity_view_off_y = 32 * 16;
void entity_view_set(int16_t x, int16_t y) {
    _entity_view_off_x = (uint16_t)(16 * 16 - (uint16_t)x * 16u);
    _entity_view_off_y = (uint16_t)(32 * 16 - (uint16_t)y * 16u);
}

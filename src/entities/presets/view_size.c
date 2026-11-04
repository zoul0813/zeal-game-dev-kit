#include <zgdk/entities/sprite.h>
extern uint16_t _entity_view_limit_x, _entity_view_limit_y;
zos_err_t entity_view_size(uint16_t width, uint16_t height) {
    if (!width || width > 640 || !height || height > 480) return ERR_INVALID_PARAMETER;
    _entity_view_limit_x = width + 15;
    _entity_view_limit_y = height + 15;
    return ERR_SUCCESS;
}

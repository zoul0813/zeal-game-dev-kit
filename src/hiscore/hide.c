#include "zgdk/tilemap.h"
#include "internal.h"

void hiscore_hide(void) {
    tilemap_fill(hiscore_config->context, hiscore_config->layer, hiscore_config->empty_tile, 0, 0, hiscore_config->width, hiscore_config->height);
}

#include <core.h>
#include "zgdk/tilemap.h"
#include "zgdk/hud/tilemap.h"
#include "ui.h"

static void format_hiscore(const highscore_t *score, char *buffer) {
    mem_cpy(buffer, score->initials, 3);
    buffer[3] = ' ';
    buffer[4] = ' ';
    itoa_pad(score->score, &buffer[5], 10, 'A', '0', 5);
}

void hiscore_show(void) {
    char buffer[16];
    uint8_t offset_x = hiscore_offset_x();
    tilemap_fill(hiscore_config->context, hiscore_config->layer, hiscore_config->empty_tile, 0, 0, hiscore_config->width, hiscore_config->height);
    uint8_t i;
    for(i = 0; i < HISCORES_COUNT; i++) {
        highscore_t *score = &hiscore_table[i];
        format_hiscore(score, buffer);
        hud_tilemap_nput_s_layer(hiscore_config->context, buffer, 10, hiscore_config->layer, offset_x, OFFSET_Y + i);
    }
}

#include <core.h>
#include "zgdk/input.h"
#include "zgdk/tilemap.h"
#include "zgdk/utils/print.h"
#include "ui.h"

#define LEFT1     (input1 & BUTTON_LEFT)
#define RIGHT1    (input1 & BUTTON_RIGHT)
#define UP1       (input1 & BUTTON_UP)
#define DOWN1     (input1 & BUTTON_DOWN)
#define BUTTON1_B (input1 & BUTTON_B)

int8_t hiscore_add(uint16_t score) {
    // find index of score
    uint8_t i, j;
    highscore_t *hiscore;
    for(i = 0; i < HISCORES_COUNT; i++) {
        hiscore = &hiscore_table[i];
        if(score > hiscore->score) break; // we found your rank
    }
    if(i >= HISCORES_COUNT) return -1; // sorry, you didn't make the table!

    // move everything else down, drop the lowest
    for(j = HISCORES_COUNT - 1; j > i; j--) {
        mem_cpy(&hiscore_table[j], &hiscore_table[j-1], sizeof(highscore_t));
    }

    // update the table with your score
    hiscore->score = score;
    mem_cpy(&hiscore->initials, &hiscore_last_initials, 3);

    hiscore_show();

    uint8_t _frames = 0, charindex = 0;
    char c = hiscore->initials[charindex];
    uint8_t offset_x = hiscore_offset_x();
    uint16_t last_input = 0;
    char blink;
    while(1) {
        gfx_wait_vblank(hiscore_config->context);
        if(_frames > 60) _frames = 0;
        if(_frames > 30) {
            blink = ' ';
        } else {
            blink = c;
        }
        nprint_string_layer(hiscore_config->context, &blink, 1, hiscore_config->layer, offset_x + charindex, OFFSET_Y + i);

        _frames++;
        gfx_wait_end_vblank(hiscore_config->context);

        uint16_t input1 = input_read();

        if(input1 != last_input) {
            if(LEFT1 || RIGHT1) {
                nprint_string_layer(hiscore_config->context, &c, 1, hiscore_config->layer, offset_x + charindex, OFFSET_Y + i);
                if (LEFT1)  charindex--;
                if (RIGHT1) charindex++;
                if(charindex == 255) charindex = 2;
                if(charindex > 2) charindex = 0;
                c = hiscore->initials[charindex];
            }

            if (UP1)    c++;
            if (DOWN1)  c--;

            if(c > 90) c = 65;
            if(c < 65) c = 90;
            hiscore->initials[charindex] = c;

            if (BUTTON1_B) break; // we're done!
        }
        last_input = input1;
    }

    // copy the last initials for the next round
    mem_cpy(&hiscore_last_initials, &hiscore->initials, 3);

    hiscore_show();
    msleep(3000);
    hiscore_hide();
    if(hiscore_config->path != NULL) {
        hiscore_save(hiscore_config->path);
    }

    return i;
}

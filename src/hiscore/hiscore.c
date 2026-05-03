#include <core.h>
#include <stdint.h>
#include <zos_vfs.h>
#include "zgdk/utils/ensure_parent_dirs.h"
#include "zgdk/hiscore.h"
#include "zgdk/tilemap.h"
#include "zgdk/input.h"
#include "zgdk/utils/print.h"

#define OFFSET_Y    (3)

#define LEFT1     (input1 & BUTTON_LEFT)
#define RIGHT1    (input1 & BUTTON_RIGHT)
#define UP1       (input1 & BUTTON_UP)
#define DOWN1     (input1 & BUTTON_DOWN)
#define BUTTON1_B (input1 & BUTTON_B)
#define BUTTON1_A (input1 & BUTTON_A)
#define START1    (input1 & BUTTON_START)
#define SELECT1   (input1 & BUTTON_SELECT)

static highscore_t hiscores[HISCORES_COUNT];
static const hiscore_config_t *hiscore_config;
static char last_initials[3] = { 'A', 'A', 'A' };
static char _buffer[16];

static void format_hiscore(highscore_t *score) {
    mem_cpy(_buffer, score->initials, 3);
    _buffer[3] = ' ';
    _buffer[4] = ' ';
    itoa_pad(score->score, &_buffer[5], 10, 'A', '0', 5);
}

static uint8_t hiscore_offset_x(void) {
    return (hiscore_config->width - 10) / 2;
}

void hiscore_init(const hiscore_config_t *config) {
    hiscore_config = config;

    if(hiscore_config->default_scores != NULL) {
        mem_cpy(hiscores, hiscore_config->default_scores, sizeof(hiscores));
    } else {
        mem_set(hiscores, 0, sizeof(hiscores));
    }

    if(hiscore_config->path != NULL) {
        hiscore_load(hiscore_config->path);
    }
}

void hiscore_show(void) {
    uint8_t offset_x = hiscore_offset_x();
    tilemap_fill(hiscore_config->context, hiscore_config->layer, hiscore_config->empty_tile, 0, 0, hiscore_config->width, hiscore_config->height);
    uint8_t i;
    for(i = 0; i < HISCORES_COUNT; i++) {
        highscore_t *score = &hiscores[i];
        format_hiscore(score);
        nprint_string_layer(hiscore_config->context, _buffer, 10, hiscore_config->layer, offset_x, OFFSET_Y + i);
    }
}

void hiscore_hide(void) {
    tilemap_fill(hiscore_config->context, hiscore_config->layer, hiscore_config->empty_tile, 0, 0, hiscore_config->width, hiscore_config->height);
}

int8_t hiscore_add(uint16_t score) {
    // find index of score
    uint8_t i, j;
    highscore_t *hiscore;
    for(i = 0; i < HISCORES_COUNT; i++) {
        hiscore = &hiscores[i];
        if(score > hiscore->score) break; // we found your rank
    }
    if(i >= HISCORES_COUNT) return -1; // sorry, you didn't make the table!

    // move everything else down, drop the lowest
    for(j = HISCORES_COUNT - 1; j > i; j--) {
        hiscores[j].score = hiscores[j-1].score;
        mem_cpy(&hiscores[j], &hiscores[j-1], sizeof(highscore_t));
    }

    // update the table with your score
    hiscore->score = score;
    mem_cpy(&hiscore->initials, &last_initials, 3);

    hiscore_show();

    uint8_t _frames = 0, charindex = 0;
    char c = hiscore->initials[charindex];
    uint8_t offset_x = hiscore_offset_x();
    uint16_t input1 = 0, last_input = 0;
    while(1) {
        gfx_wait_vblank(hiscore_config->context);
        if(_frames > 60) _frames = 0;
        if(_frames > 30) {
            _buffer[0] = ' ';
        } else {
            _buffer[0] = c;
        }
        nprint_string_layer(hiscore_config->context, _buffer, 1, hiscore_config->layer, offset_x + charindex, OFFSET_Y + i);

        _frames++;
        gfx_wait_end_vblank(hiscore_config->context);

        uint16_t input1 = input_get();

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
    mem_cpy(&last_initials, &hiscore->initials, 3);

    hiscore_show();
    msleep(3000);
    hiscore_hide();
    if(hiscore_config->path != NULL) {
        hiscore_save(hiscore_config->path);
    }

    return i;
}

zos_err_t hiscore_save(const char *path) {
    zos_err_t err = ensure_parent_dirs(path);
    if(err != ERR_SUCCESS)
        return err;

    zos_dev_t dev = open(path, O_WRONLY | O_CREAT | O_TRUNC);
    if(dev < 0)
        return (zos_err_t)(-dev);

    uint16_t size = 3;
    err = write(dev, HISCORE_FORMAT, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != 3) {
        err = ERR_FAILURE;
        goto done;
    }

    uint8_t version = HISCORE_VERSION;
    size = sizeof(version);
    err = write(dev, &version, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(version)) {
        err = ERR_FAILURE;
        goto done;
    }

    size = sizeof(hiscores);
    err = write(dev, hiscores, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(hiscores)) {
        err = ERR_FAILURE;
        goto done;
    }

    size = sizeof(last_initials);
    err = write(dev, last_initials, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(last_initials))
        err = ERR_FAILURE;

done:
    close(dev);
    return err;
}

zos_err_t hiscore_load(const char *path) {
    char format[3];
    uint8_t version;
    highscore_t loaded_hiscores[HISCORES_COUNT];
    char loaded_initials[3];
    zos_err_t err;
    uint16_t size;

    zos_dev_t dev = open(path, O_RDONLY);
    if(dev < 0)
        return (zos_err_t)(-dev);

    size = 3;
    err = read(dev, format, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if((size != 3) || (mem_cmp(format, HISCORE_FORMAT, 3) != 0)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }

    size = sizeof(version);
    err = read(dev, &version, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if((size != sizeof(version)) || (version != HISCORE_VERSION)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }

    size = sizeof(loaded_hiscores);
    err = read(dev, loaded_hiscores, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(loaded_hiscores)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }

    size = sizeof(loaded_initials);
    err = read(dev, loaded_initials, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(loaded_initials)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }

    mem_cpy(hiscores, loaded_hiscores, sizeof(hiscores));
    mem_cpy(last_initials, loaded_initials, sizeof(last_initials));

done:
    close(dev);
    return err;
}

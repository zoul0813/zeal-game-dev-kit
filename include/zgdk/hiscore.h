#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>

#ifndef HISCORE_H
#define HISCORE_H

#define HISCORES_COUNT  10

#define HISCORE_FORMAT "ZHS"
#define HISCORE_VERSION 1

typedef struct {
    char initials[3];
    uint16_t score;
} highscore_t;

typedef struct {
    gfx_context *context;
    uint8_t width;
    uint8_t height;
    uint8_t layer;
    uint8_t empty_tile;
    const highscore_t *default_scores;
    const char *path;
} hiscore_config_t;

void hiscore_init(const hiscore_config_t *config);

/**
 * @brief Show the hiscore table
 */
void hiscore_show(void);

/**
 * @brief Hide the hiscore table
 */
void hiscore_hide(void);

/**
 * @brief add a score to the table
 * @return -1 if score is too low, otherwise the index on the table (ie; 0 for highest score)
 */
int8_t hiscore_add(uint16_t score);

zos_err_t hiscore_save(const char *path);

zos_err_t hiscore_load(const char *path);

#endif

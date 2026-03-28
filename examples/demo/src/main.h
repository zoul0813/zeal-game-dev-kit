#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>

#ifndef MAIN_H
#define MAIN_H

#define ACTION_NONE     0
#define ACTION_PAUSE    1
#define ACTION_CONTINUE 1
#define ACTION_QUIT     10

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define WIDTH         20
#define HEIGHT        15

#define LEVEL_LAYER 0
#define UI_LAYER    1
#define EMPTY_TILE  0

#define LEFT1   (input1 & BUTTON_LEFT)
#define RIGHT1  (input1 & BUTTON_RIGHT)
#define UP1     (input1 & BUTTON_UP)
#define DOWN1   (input1 & BUTTON_DOWN)
#define BUTTON1_B  (input1 & BUTTON_B)
#define BUTTON1_A  (input1 & BUTTON_A)
#define START1 (input1 & BUTTON_START)
#define SELECT1 (input1 & BUTTON_SELECT)

typedef uint8_t error;

void on_error(zos_err_t err);
void init(void);
void deinit(void);
void reset(void);
uint8_t input(void);
error load_level(uint8_t which);

void update(void);
void draw_paused(uint8_t paused);
void draw_gameover(uint8_t gameover);
void draw(void);

extern gfx_context vctx;

#endif
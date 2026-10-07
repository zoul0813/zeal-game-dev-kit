#pragma once

#include "zgdk/sound/tracker.h"

extern uint8_t zmt_ticks;
extern uint8_t zmt_current_pattern;
extern uint8_t zmt_current_arrangement;
extern uint8_t zmt_next_step;
extern uint8_t zmt_last_step;

void tracker_log_io_error(const char* operation, uint8_t step, zos_err_t err);
void tracker_log_open_error(const char* message, const char* filename, zos_err_t err);
void tracker_copy_title_field(char* dst, const char* src);

zos_err_t tracker_write_exact(zos_dev_t dev, const void *data, uint16_t size);

/* Shared cursor for ROM validation and decoding. NULL output validates only. */
typedef struct {
    const uint8_t *pos;
    uint16_t remaining;
} tracker_rom_cursor_t;

uint8_t tracker_rom_read(tracker_rom_cursor_t *cursor, void *out, uint16_t size);

uint8_t tracker_rom_arrangement(tracker_rom_cursor_t *cursor, arrangement_t *out, uint8_t count);
uint8_t tracker_rom_pattern(tracker_rom_cursor_t *cursor, pattern_t *out);

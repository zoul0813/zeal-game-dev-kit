#pragma once

#include "zgdk/sound/tracker.h"

extern uint8_t ticks;
extern uint8_t current_pattern;
extern uint8_t current_arrangement;
extern uint8_t next_step;
extern uint8_t last_step;

void tracker_log_io_error(const char* operation, uint8_t step, zos_err_t err);
void tracker_log_open_error(const char* message, const char* filename, zos_err_t err);
void tracker_copy_title_field(char* dst, const char* src);

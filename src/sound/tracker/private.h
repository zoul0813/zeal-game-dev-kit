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

void zmt_voice_active(sound_voice_t voice, uint8_t active);
void zmt_voice_volume(sound_voice_t voice, sound_volume_t volume);

/* File I/O stays in separate objects so ROM playback does not link it. */
zos_err_t tracker_read_exact(zos_dev_t dev, void* buffer, uint16_t length);
zos_err_t tracker_write_exact(zos_dev_t dev, const void* buffer, uint16_t length);

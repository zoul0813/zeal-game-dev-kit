#include <stdint.h>
#include "sdcc.h"

#ifndef MUSIC_H
#define MUSIC_H


// 16k / 4
#ifndef MAX_RECORDS
#define MAX_RECORDS   4096
#endif
#define FREQ_NONE     0

typedef enum {
  T_NONE    = 0,
  T_PLAY    = 1,
  T_RECORD  = 2,
} music_state_t;

typedef struct {
  uint16_t frame;
  uint16_t freq;
  uint8_t voice_wave;
} __attribute__((packed)) Record;

typedef struct {
  music_state_t state;
  uint16_t length;
  Record records[MAX_RECORDS];
} __attribute__((packed)) Track;


zos_err_t music_load_from_file(const char* path, Track *track);
// Track* music_get(void);
zos_err_t music_init(Track *track);
void music_transport(music_state_t state, uint16_t frame);
void music_store(Record *record);
void music_tick(void);
void music_loop(uint8_t loop);

music_state_t music_state(void);
Record* music_next(uint8_t tick);
Record* music_at(uint16_t position);
uint16_t music_frame(void);

uint16_t music_length(void);
uint16_t music_pos(void);

static uint8_t music_get_voice(uint8_t voice_wave) {
  return (voice_wave & 0xF0) >> 4;
}

static uint8_t music_get_wave(uint8_t voice_wave) {
  return (voice_wave & 0x0F);
}

static uint8_t music_set_voice_wave(uint8_t voice, uint8_t wave) {
  return (voice << 4) | (wave);
}

static uint8_t music_end(Record *record) {
  return (record->freq == 0xFF) && (record->voice_wave == 0xFF);
}

#endif
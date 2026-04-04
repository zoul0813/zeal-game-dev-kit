#include <stdint.h>
#include <zos_errors.h>
#include <zvb_sound.h>
#include "zgdk/sound/sounds.h"
#include "private.h"

zos_err_t music_init(Track *track) {
  if(track == NULL) return ERR_INVALID_PARAMETER;
  _track = track;
  _track->state = T_NONE;
  music_position = 0;
  _track->length = 0;

  // TODO: can we just initialize the first element as an "END" record?
  _track->records[0].freq = 0xFF;
  _track->records[0].voice_wave = 0xFF;

  // for(uint16_t i = 0; i < MAX_RECORDS; i++) {
  //   Record record = { 0, 0xFF, 0xFF };
  //   mem_cpy(&_track->records[i], &record, sizeof(Record));
  // }

  zvb_sound_initialize(1);

  return ERR_SUCCESS;
}

void music_tick(void) {
  music_position++;
  if(music_position >= MAX_RECORDS) {
    music_position = MAX_RECORDS - 1;
    uint16_t frame = _track->records[music_position].frame;
    music_transport(T_NONE, frame);
  }
}

Record* music_next(uint8_t tick) {
  Record *record = &_track->records[music_position];
  if(tick) music_tick();
  return record;
}

void music_transport(music_state_t state, uint16_t frame) {
  // if we're recordiong, or playing, we stop
  if(state == _track->state) state = T_NONE;

  switch(_track->state) {
    case T_RECORD:
      // add the "END" record
      Record record = {
          .frame = frame,
          .freq = 0xFF,
          .voice_wave = 0xFF,
      };
      music_store(&record);
      break;
  }

  // music_position = 0; // maybe
  _track->state = state;

  switch(state) {
    case T_PLAY:
      ticks = frame;
      if(frame > 0) {
        Record *prev = NULL;
        for(uint16_t i = 0; i < MAX_RECORDS; i++) {
          Record *record = music_at(i);
          // found a matching frame
          if(record->frame == frame) {
            music_position = i;
            break;
          }
          // requested frame is between prev/next, so use prev
          if(record->frame > frame && prev != NULL) {
            music_position = i--; //prev->frame;
            break;
          }
          // // we found the last record, so stop
          if(music_end(record)) {
            music_position = i;
            break;
          }
          prev = record;
        }
      } else {
        music_position = 0;
      }
      break;
    case T_NONE:
      zvb_sound_set_voices(VOICEALL, 0, 0);
      break;
  }
}

void music_loop(uint8_t loop) {
    if(_track->state != T_PLAY) return;
    Record *record = music_next(0);
    ticks++;
    do {
        if(ticks >= record->frame) {
            uint16_t frame = record->frame;
            uint16_t freq = record->freq;
            uint8_t voice_wave = record->voice_wave;
            uint8_t voice = music_get_voice(voice_wave); // ((voice_wave & 0xF0) >> 4);
            uint8_t wave = music_get_wave(voice_wave); // (voice_wave & 0x0F);

            music_tick();
            if(voice_wave == 0xFF) {
              if(loop) {
                ticks = 0;
                music_position = 0;
              } else {
                music_transport(T_NONE, ticks);
              }
              return;
            } else {
                if(wave >= 0x0F) {
                    zvb_sound_set_voices((1 << voice), freq, WAV_SQUARE);
                } else {
                    zvb_sound_set_voices((1 << voice), freq, wave);
                }
                record = music_next(0);
            }
        } else {
            record = NULL;
        }
    } while(record != NULL);
}

#include "zgdk/sound/music.h"

uint8_t music_end(Record *record) {
  return (record->freq == 0xFF) && (record->voice_wave == 0xFF);
}

#include "zgdk/sound/music.h"

uint8_t music_get_voice(uint8_t voice_wave) {
  return (voice_wave & 0xF0) >> 4;
}

#include "zgdk/sound/music.h"

uint8_t music_set_voice_wave(uint8_t voice, uint8_t wave) {
  return (voice << 4) | (wave);
}

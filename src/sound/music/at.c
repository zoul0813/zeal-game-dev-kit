#include "private.h"
Record* music_at(uint16_t position) {
  if(position < _track->length) {
    return &_track->records[position];
  }
  return NULL;
}

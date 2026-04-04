#include <core.h>
#include "private.h"
void music_store(Record *record) {
  mem_cpy(&_track->records[music_position], record, sizeof(Record));
  music_tick();
  _track->length = music_position;
}
